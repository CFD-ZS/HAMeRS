#include "flow_model/Riemann_solver/RiemannSolverHLLC_HLL.hpp"

#include <cmath>

/*
 * Compute the fluxes and and velocities at the intercell faces
 * for single-species flow model.
 */
void
RiemannSolverHLLC_HLL::computeIntercellFluxForSingleSpecies(
    std::vector<double*> flux_intercell,
    const double* const density_L,
    const double* const density_R,
    const std::vector<const double*> momentum_L,
    const std::vector<const double*> momentum_R,
    const double* const total_energy_L,
    const double* const total_energy_R,
    DIRECTION direction)
{
    if (d_dim == tbox::Dimension(1))
    {
#ifdef DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(static_cast<int>(flux_intercell.size()) == d_num_eqn);
        TBOX_ASSERT(static_cast<int>(momentum_L.size()) == 1);
        TBOX_ASSERT(static_cast<int>(momentum_R.size()) == 1);
        
        for (int fi = 0; fi < static_cast<int>(flux_intercell.size()); fi++)
        {
            TBOX_ASSERT(flux_intercell[fi]);
        }
        TBOX_ASSERT(density_L);
        TBOX_ASSERT(density_R);
        for (int mi = 0; mi < static_cast<int>(momentum_L.size()); mi++)
        {
            TBOX_ASSERT(momentum_L[mi]);
        }
        for (int mi = 0; mi < static_cast<int>(momentum_R.size()); mi++)
        {
            TBOX_ASSERT(momentum_R[mi]);
        }
        TBOX_ASSERT(total_energy_L);
        TBOX_ASSERT(total_energy_R);
#endif
        
        switch (direction)
        {
            case X_DIRECTION:
            {
                std::vector<double*> F_x_intercell = flux_intercell;
                
                const double& rho_L = *density_L;
                const double& rho_R = *density_R;
                
                const double& rho_u_L = *(momentum_L[0]);
                const double& rho_u_R = *(momentum_R[0]);
                
                const std::vector<const double*> m_L = momentum_L;
                const std::vector<const double*> m_R = momentum_R;
                
                const double& u_L = rho_u_L/rho_L;
                const double& u_R = rho_u_R/rho_R;
                
                const double& E_L = *total_energy_L;
                const double& E_R = *total_energy_R;
                
                const double p_L = d_equation_of_state->getPressure(
                    &rho_L,
                    m_L,
                    &E_L);
                
                const double p_R = d_equation_of_state->getPressure(
                    &rho_R,
                    m_R,
                    &E_R);
                
                const double c_L = d_equation_of_state->getSoundSpeedWithPressure(
                    &rho_L,
                    &p_L);
                
                const double c_R = d_equation_of_state->getSoundSpeedWithPressure(
                    &rho_R,
                    &p_R);
                
                const double u_average = 0.5*(u_L + u_R);
                const double c_average = 0.5*(c_L + c_R);
                
                const double s_L = fmin(u_average - c_average, u_L - c_L);
                const double s_R = fmax(u_average + c_average, u_R + c_R);
                
                const double s_minus = fmin(0.0, s_L);
                const double s_plus  = fmax(0.0, s_R);
                
                const double s_star =
                    (p_R - p_L + rho_L*u_L*(s_L - u_L) - rho_R*u_R*(s_R - u_R))/(rho_L*(s_L - u_L) -
                        rho_R*(s_R - u_R));
                
                if (s_star > 0)
                {
                    const double Chi_star_L = (s_L - u_L)/(s_L - s_star);
                    
                    std::vector<double> Q_star_L;
                    Q_star_L.push_back(Chi_star_L*rho_L);
                    Q_star_L.push_back(Chi_star_L*rho_L*s_star);
                    Q_star_L.push_back(Chi_star_L*(E_L + (s_star - u_L)*(rho_L*s_star + p_L/(s_L - u_L))));
                    
                    std::vector<const double*> Q_L;
                    Q_L.push_back(&rho_L);
                    Q_L.push_back(m_L[0]);
                    Q_L.push_back(&E_L);
                    
                    std::vector<double> F_x_L;
                    F_x_L.push_back(rho_u_L);
                    F_x_L.push_back(rho_u_L*u_L + p_L);
                    F_x_L.push_back(u_L*(E_L + p_L));
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        *(F_x_intercell[ei]) = F_x_L[ei] + s_minus*(Q_star_L[ei] - *(Q_L[ei]));
                    }
                }
                else
                {
                    const double Chi_star_R = (s_R - u_R)/(s_R - s_star);
                    
                    std::vector<double> Q_star_R;
                    Q_star_R.push_back(Chi_star_R*rho_R);
                    Q_star_R.push_back(Chi_star_R*rho_R*s_star);
                    Q_star_R.push_back(Chi_star_R*(E_R + (s_star - u_R)*(rho_R*s_star + p_R/(s_R - u_R))));
                    
                    std::vector<const double*> Q_R;
                    Q_R.push_back(&rho_R);
                    Q_R.push_back(m_R[0]);
                    Q_R.push_back(&E_R);
                    
                    std::vector<double> F_x_R;
                    F_x_R.push_back(rho_u_R);
                    F_x_R.push_back(rho_u_R*u_R + p_R);
                    F_x_R.push_back(u_R*(E_R + p_R));
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        *(F_x_intercell[ei]) = F_x_R[ei] + s_plus*(Q_star_R[ei] - *(Q_R[ei]));
                    }
                }
                
                break;
            }
            case Y_DIRECTION:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "There is no y direction for 1D problem."
                           << std::endl);
                break;
            }
            case Z_DIRECTION:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "There is no z direction for 1D problem."
                           << std::endl);
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "Unknown direction."
                           << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(2))
    {
#ifdef DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(static_cast<int>(flux_intercell.size()) == d_num_eqn);
        TBOX_ASSERT(static_cast<int>(momentum_L.size()) == 2);
        TBOX_ASSERT(static_cast<int>(momentum_R.size()) == 2);
        
        for (int fi = 0; fi < static_cast<int>(flux_intercell.size()); fi++)
        {
            TBOX_ASSERT(flux_intercell[fi]);
        }
        TBOX_ASSERT(density_L);
        TBOX_ASSERT(density_R);
        for (int mi = 0; mi < static_cast<int>(momentum_L.size()); mi++)
        {
            TBOX_ASSERT(momentum_L[mi]);
        }
        for (int mi = 0; mi < static_cast<int>(momentum_R.size()); mi++)
        {
            TBOX_ASSERT(momentum_R[mi]);
        }
        TBOX_ASSERT(total_energy_L);
        TBOX_ASSERT(total_energy_R);
#endif
        
        switch (direction)
        {
            case X_DIRECTION:
            {
                std::vector<double*> F_x_intercell = flux_intercell;
                
                std::vector<double> F_x_intercell_HLLC;
                std::vector<double> F_x_intercell_HLL;
                
                const double& rho_L = *density_L;
                const double& rho_R = *density_R;
                
                const double& rho_u_L = *(momentum_L[0]);
                const double& rho_u_R = *(momentum_R[0]);
                
                const double& rho_v_L = *(momentum_L[1]);
                const double& rho_v_R = *(momentum_R[1]);
                
                const double& u_L = rho_u_L/rho_L;
                const double& u_R = rho_u_R/rho_R;
                
                const double& v_L = rho_v_L/rho_L;
                const double& v_R = rho_v_R/rho_R;
                
                const std::vector<const double*> m_L = momentum_L;
                const std::vector<const double*> m_R = momentum_R;
                
                const double& E_L = *total_energy_L;
                const double& E_R = *total_energy_R;
                
                const double p_L = d_equation_of_state->getPressure(
                    &rho_L,
                    m_L,
                    &E_L);
                
                const double p_R = d_equation_of_state->getPressure(
                    &rho_R,
                    m_R,
                    &E_R);
                
                const double c_L = d_equation_of_state->getSoundSpeedWithPressure(
                    &rho_L,
                    &p_L);
                
                const double c_R = d_equation_of_state->getSoundSpeedWithPressure(
                    &rho_R,
                    &p_R);
                
                const double u_average = 0.5*(u_L + u_R);
                const double c_average = 0.5*(c_L + c_R);
                
                const double s_L = fmin(u_average - c_average, u_L - c_L);
                const double s_R = fmax(u_average + c_average, u_R + c_R);
                
                const double s_minus = fmin(0.0, s_L);
                const double s_plus  = fmax(0.0, s_R);
                
                const double s_star =
                    (p_R - p_L + rho_L*u_L*(s_L - u_L) - rho_R*u_R*(s_R - u_R))/(rho_L*(s_L - u_L) -
                        rho_R*(s_R - u_R));
                
                if (s_star > 0)
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_L = (s_L - u_L)/(s_L - s_star);
                    
                    std::vector<double> Q_star_L;
                    Q_star_L.push_back(Chi_star_L*rho_L);
                    Q_star_L.push_back(Chi_star_L*rho_L*s_star);
                    Q_star_L.push_back(Chi_star_L*rho_v_L);
                    Q_star_L.push_back(Chi_star_L*(E_L + (s_star - u_L)*(rho_L*s_star + p_L/(s_L - u_L))));
                    
                    std::vector<const double*> Q_L;
                    Q_L.push_back(&rho_L);
                    Q_L.push_back(m_L[0]);
                    Q_L.push_back(m_L[1]);
                    Q_L.push_back(&E_L);
                    
                    std::vector<double> F_x_L;
                    F_x_L.push_back(rho_u_L);
                    F_x_L.push_back(rho_u_L*u_L + p_L);
                    F_x_L.push_back(rho_u_L*v_L);
                    F_x_L.push_back(u_L*(E_L + p_L));
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_x_intercell_HLLC.push_back(F_x_L[ei] + s_minus*(Q_star_L[ei] - *(Q_L[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_L >= 0)
                    {
                        F_x_intercell_HLL = F_x_L;
                    }
                    else
                    {
                        std::vector<double> F_x_R;
                        F_x_R.push_back(rho_u_R);
                        F_x_R.push_back(rho_u_R*u_R + p_R);
                        F_x_R.push_back(rho_u_R*v_R);
                        F_x_R.push_back(u_R*(E_R + p_R));
                        
                        if (s_R <= 0)
                        {
                            F_x_intercell_HLL = F_x_R;
                        }
                        else
                        {
                            std::vector<const double*> Q_R;
                            Q_R.push_back(&rho_R);
                            Q_R.push_back(m_R[0]);
                            Q_R.push_back(m_R[1]);
                            Q_R.push_back(&E_R);
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_x_intercell_HLL.push_back((s_R*F_x_L[ei] - s_L*F_x_R[ei] +
                                    s_R*s_L*(*(Q_R[ei]) - *(Q_L[ei])))/(s_R - s_L));
                            }
                        }
                    }
                }
                else
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_R = (s_R - u_R)/(s_R - s_star);
                    
                    std::vector<double> Q_star_R;
                    Q_star_R.push_back(Chi_star_R*rho_R);
                    Q_star_R.push_back(Chi_star_R*rho_R*s_star);
                    Q_star_R.push_back(Chi_star_R*rho_v_R);
                    Q_star_R.push_back(Chi_star_R*(E_R + (s_star - u_R)*(rho_R*s_star + p_R/(s_R - u_R))));
                    
                    std::vector<const double*> Q_R;
                    Q_R.push_back(&rho_R);
                    Q_R.push_back(m_R[0]);
                    Q_R.push_back(m_R[1]);
                    Q_R.push_back(&E_R);
                    
                    std::vector<double> F_x_R;
                    F_x_R.push_back(rho_u_R);
                    F_x_R.push_back(rho_u_R*u_R + p_R);
                    F_x_R.push_back(rho_u_R*v_R);
                    F_x_R.push_back(u_R*(E_R + p_R));
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_x_intercell_HLLC.push_back(F_x_R[ei] + s_plus*(Q_star_R[ei] - *(Q_R[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_R <= 0)
                    {
                        F_x_intercell_HLL = F_x_R;
                    }
                    else
                    {
                        std::vector<double> F_x_L;
                        F_x_L.push_back(rho_u_L);
                        F_x_L.push_back(rho_u_L*u_L + p_L);
                        F_x_L.push_back(rho_u_L*v_L);
                        F_x_L.push_back(u_L*(E_L + p_L));
                        
                        if (s_L >= 0)
                        {
                            F_x_intercell_HLL = F_x_L;
                        }
                        else
                        {
                            std::vector<const double*> Q_L;
                            Q_L.push_back(&rho_L);
                            Q_L.push_back(m_L[0]);
                            Q_L.push_back(m_L[1]);
                            Q_L.push_back(&E_L);
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_x_intercell_HLL.push_back((s_R*F_x_L[ei] - s_L*F_x_R[ei] +
                                    s_R*s_L*(*(Q_R[ei]) - *(Q_L[ei])))/(s_R - s_L));
                            }
                        }
                    }
                }
                
                /*
                 * Calulate the weights beta for hybridization.
                 */
                
                double alpha1, alpha2;
                
                double vel_mag = sqrt(pow(u_R - u_L, 2) + pow(v_R - v_L, 2));
                
                if (vel_mag < 1e-40)
                {
                    alpha1 = 1.0;
                    alpha2 = 0.0;
                }
                else
                {
                    alpha1 = fabs(u_R - u_L)/vel_mag;
                    alpha2 = sqrt(1.0 - alpha1*alpha1);
                }
                
                double beta1 = 0.5 + 0.5*alpha1/(alpha1 + alpha2);
                double beta2 = 1.0 - beta1;
                
                /*
                 * Compute the HLLC-HLL flux.
                 */
                
                *(F_x_intercell[0]) = beta1*F_x_intercell_HLLC[0] + beta2*F_x_intercell_HLL[0];
                *(F_x_intercell[1]) = F_x_intercell_HLLC[1];
                *(F_x_intercell[2]) = beta1*F_x_intercell_HLLC[2] + beta2*F_x_intercell_HLL[2];
                *(F_x_intercell[3]) = F_x_intercell_HLLC[3];
                
                break;
            }
            case Y_DIRECTION:
            {
                std::vector<double*> F_y_intercell = flux_intercell;
                
                std::vector<double> F_y_intercell_HLLC;
                std::vector<double> F_y_intercell_HLL;
                
                const double& rho_B = *density_L;
                const double& rho_T = *density_R;
                
                const double& rho_u_B = *(momentum_L[0]);
                const double& rho_u_T = *(momentum_R[0]);
                
                const double& rho_v_B = *(momentum_L[1]);
                const double& rho_v_T = *(momentum_R[1]);
                
                const std::vector<const double*> m_B = momentum_L;
                const std::vector<const double*> m_T = momentum_R;
                
                const double& u_B = rho_u_B/rho_B;
                const double& u_T = rho_u_T/rho_T;
                
                const double& v_B = rho_v_B/rho_B;
                const double& v_T = rho_v_T/rho_T;
                
                const double& E_B = *total_energy_L;
                const double& E_T = *total_energy_R;
                
                const double p_B = d_equation_of_state->getPressure(
                    &rho_B,
                    m_B,
                    &E_B);
                
                const double p_T = d_equation_of_state->getPressure(
                    &rho_T,
                    m_T,
                    &E_T);
                
                const double c_B = d_equation_of_state->getSoundSpeedWithPressure(
                    &rho_B,
                    &p_B);
                
                const double c_T = d_equation_of_state->getSoundSpeedWithPressure(
                    &rho_T,
                    &p_T);
                
                const double v_average = 0.5*(v_B + v_T);
                const double c_average = 0.5*(c_B + c_T);
                
                const double s_B = fmin(v_average - c_average, v_B - c_B);
                const double s_T = fmax(v_average + c_average, v_T + c_T);
                
                const double s_minus = fmin(0.0, s_B);
                const double s_plus  = fmax(0.0, s_T);
                
                const double s_star =
                    (p_T - p_B + rho_B*v_B*(s_B - v_B) - rho_T*v_T*(s_T - v_T))/(rho_B*(s_B - v_B) -
                        rho_T*(s_T - v_T));
                
                if (s_star > 0)
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_B = (s_B - v_B)/(s_B - s_star);
                    
                    std::vector<double> Q_star_B;
                    Q_star_B.push_back(Chi_star_B*rho_B);
                    Q_star_B.push_back(Chi_star_B*rho_u_B);
                    Q_star_B.push_back(Chi_star_B*rho_B*s_star);
                    Q_star_B.push_back(Chi_star_B*(E_B + (s_star - v_B)*(rho_B*s_star + p_B/(s_B - v_B))));
                    
                    std::vector<const double*> Q_B;
                    Q_B.push_back(&rho_B);
                    Q_B.push_back(m_B[0]);
                    Q_B.push_back(m_B[1]);
                    Q_B.push_back(&E_B);
                    
                    std::vector<double> F_y_B;
                    F_y_B.push_back(rho_v_B);
                    F_y_B.push_back(rho_v_B*u_B);
                    F_y_B.push_back(rho_v_B*v_B + p_B);
                    F_y_B.push_back(v_B*(E_B + p_B));
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_y_intercell_HLLC.push_back(F_y_B[ei] + s_minus*(Q_star_B[ei] - *(Q_B[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_B >= 0)
                    {
                        F_y_intercell_HLL = F_y_B;
                    }
                    else
                    {
                        std::vector<double> F_y_T;
                        F_y_T.push_back(rho_v_T);
                        F_y_T.push_back(rho_v_T*u_T);
                        F_y_T.push_back(rho_v_T*v_T + p_T);
                        F_y_T.push_back(v_T*(E_T + p_T));
                        
                        if (s_T <= 0)
                        {
                            F_y_intercell_HLL = F_y_T;
                        }
                        else
                        {
                            std::vector<const double*> Q_T;
                            Q_T.push_back(&rho_T);
                            Q_T.push_back(m_T[0]);
                            Q_T.push_back(m_T[1]);
                            Q_T.push_back(&E_T);
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_y_intercell_HLL.push_back((s_T*F_y_B[ei] - s_B*F_y_T[ei] +
                                    s_T*s_B*(*(Q_T[ei]) - *(Q_B[ei])))/(s_T - s_B));
                            }
                        }
                    }
                }
                else
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_T = (s_T - v_T)/(s_T - s_star);
                    
                    std::vector<double> Q_star_T;
                    Q_star_T.push_back(Chi_star_T*rho_T);
                    Q_star_T.push_back(Chi_star_T*rho_u_T);
                    Q_star_T.push_back(Chi_star_T*rho_T*s_star);
                    Q_star_T.push_back(Chi_star_T*(E_T + (s_star - v_T)*(rho_T*s_star + p_T/(s_T - v_T))));
                    
                    std::vector<const double*> Q_T;
                    Q_T.push_back(&rho_T);
                    Q_T.push_back(m_T[0]);
                    Q_T.push_back(m_T[1]);
                    Q_T.push_back(&E_T);
                    
                    std::vector<double> F_y_T;
                    F_y_T.push_back(rho_v_T);
                    F_y_T.push_back(rho_v_T*u_T);
                    F_y_T.push_back(rho_v_T*v_T + p_T);
                    F_y_T.push_back(v_T*(E_T + p_T));
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_y_intercell_HLLC.push_back(F_y_T[ei] + s_plus*(Q_star_T[ei] - *(Q_T[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_T <= 0)
                    {
                        F_y_intercell_HLL = F_y_T;
                    }
                    else
                    {
                        std::vector<double> F_y_B;
                        F_y_B.push_back(rho_v_B);
                        F_y_B.push_back(rho_v_B*u_B);
                        F_y_B.push_back(rho_v_B*v_B + p_B);
                        F_y_B.push_back(v_B*(E_B + p_B));
                        
                        if (s_B >= 0)
                        {
                            F_y_intercell_HLL = F_y_B;
                        }
                        else
                        {
                            std::vector<const double*> Q_B;
                            Q_B.push_back(&rho_B);
                            Q_B.push_back(m_B[0]);
                            Q_B.push_back(m_B[1]);
                            Q_B.push_back(&E_B);
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_y_intercell_HLL.push_back((s_T*F_y_B[ei] - s_B*F_y_T[ei] +
                                    s_T*s_B*(*(Q_T[ei]) - *(Q_B[ei])))/(s_T - s_B));
                            }
                        }
                    }
                }
                
                /*
                 * Calulate the weights beta for hybridization.
                 */
                
                double alpha1, alpha2;
                
                double vel_mag = sqrt(pow(u_T - u_B, 2) + pow(v_T - v_B, 2));
                
                if (vel_mag < 1e-40)
                {
                    alpha1 = 1.0;
                    alpha2 = 0.0;
                }
                else
                {
                    alpha1 = fabs(v_T - v_B)/vel_mag;
                    alpha2 = sqrt(1.0 - alpha1*alpha1);
                }
                
                double beta1 = 0.5 + 0.5*alpha1/(alpha1 + alpha2);
                double beta2 = 1.0 - beta1;
                
                /*
                 * Compute the HLLC-HLL flux.
                 */
                
                *(F_y_intercell[0]) = beta1*F_y_intercell_HLLC[0] + beta2*F_y_intercell_HLL[0];
                *(F_y_intercell[1]) = beta1*F_y_intercell_HLLC[1] + beta2*F_y_intercell_HLL[1];
                *(F_y_intercell[2]) = F_y_intercell_HLLC[2];
                *(F_y_intercell[3]) = F_y_intercell_HLLC[3];
                
                break;
            }
            case Z_DIRECTION:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "There is no z direction for 1D problem."
                           << std::endl);
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "Unknown direction."
                           << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(3))
    {
#ifdef DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(static_cast<int>(flux_intercell.size()) == d_num_eqn);
        TBOX_ASSERT(static_cast<int>(momentum_L.size()) == 3);
        TBOX_ASSERT(static_cast<int>(momentum_R.size()) == 3);
        
        for (int fi = 0; fi < static_cast<int>(flux_intercell.size()); fi++)
        {
            TBOX_ASSERT(flux_intercell[fi]);
        }
        TBOX_ASSERT(density_L);
        TBOX_ASSERT(density_R);
        for (int mi = 0; mi < static_cast<int>(momentum_L.size()); mi++)
        {
            TBOX_ASSERT(momentum_L[mi]);
        }
        for (int mi = 0; mi < static_cast<int>(momentum_R.size()); mi++)
        {
            TBOX_ASSERT(momentum_R[mi]);
        }
        TBOX_ASSERT(total_energy_L);
        TBOX_ASSERT(total_energy_R);
#endif
        
        switch (direction)
        {
            case X_DIRECTION:
            {
                std::vector<double*> F_x_intercell = flux_intercell;
                
                std::vector<double> F_x_intercell_HLLC;
                std::vector<double> F_x_intercell_HLL;
                
                const double& rho_L = *density_L;
                const double& rho_R = *density_R;
                
                const double& rho_u_L = *(momentum_L[0]);
                const double& rho_u_R = *(momentum_R[0]);
                
                const double& rho_v_L = *(momentum_L[1]);
                const double& rho_v_R = *(momentum_R[1]);
                
                const double& rho_w_L = *(momentum_L[2]);
                const double& rho_w_R = *(momentum_R[2]);
                
                const std::vector<const double*> m_L = momentum_L;
                const std::vector<const double*> m_R = momentum_R;
                
                const double& u_L = rho_u_L/rho_L;
                const double& u_R = rho_u_R/rho_R;
                
                const double& v_L = rho_v_L/rho_L;
                const double& v_R = rho_v_R/rho_R;
                
                const double& w_L = rho_w_L/rho_L;
                const double& w_R = rho_w_R/rho_R;
                
                const double& E_L = *total_energy_L;
                const double& E_R = *total_energy_R;
                
                const double p_L = d_equation_of_state->getPressure(
                    &rho_L,
                    m_L,
                    &E_L);
                
                const double p_R = d_equation_of_state->getPressure(
                    &rho_R,
                    m_R,
                    &E_R);
                
                const double c_L = d_equation_of_state->getSoundSpeedWithPressure(
                    &rho_L,
                    &p_L);
                
                const double c_R = d_equation_of_state->getSoundSpeedWithPressure(
                    &rho_R,
                    &p_R);
                
                const double u_average = 0.5*(u_L + u_R);
                const double c_average = 0.5*(c_L + c_R);
                
                const double s_L = fmin(u_average - c_average, u_L - c_L);
                const double s_R = fmax(u_average + c_average, u_R + c_R);
                
                const double s_minus = fmin(0.0, s_L);
                const double s_plus  = fmax(0.0, s_R);
                
                const double s_star =
                    (p_R - p_L + rho_L*u_L*(s_L - u_L) - rho_R*u_R*(s_R - u_R))/(rho_L*(s_L - u_L) -
                        rho_R*(s_R - u_R));
                
                if (s_star > 0)
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_L = (s_L - u_L)/(s_L - s_star);
                    
                    std::vector<double> Q_star_L;
                    Q_star_L.push_back(Chi_star_L*rho_L);
                    Q_star_L.push_back(Chi_star_L*rho_L*s_star);
                    Q_star_L.push_back(Chi_star_L*rho_v_L);
                    Q_star_L.push_back(Chi_star_L*rho_w_L);
                    Q_star_L.push_back(Chi_star_L*(E_L + (s_star - u_L)*(rho_L*s_star + p_L/(s_L - u_L))));
                    
                    std::vector<const double*> Q_L;
                    Q_L.push_back(&rho_L);
                    Q_L.push_back(m_L[0]);
                    Q_L.push_back(m_L[1]);
                    Q_L.push_back(m_L[2]);
                    Q_L.push_back(&E_L);
                    
                    std::vector<double> F_x_L;
                    F_x_L.push_back(rho_u_L);
                    F_x_L.push_back(rho_u_L*u_L + p_L);
                    F_x_L.push_back(rho_u_L*v_L);
                    F_x_L.push_back(rho_u_L*w_L);
                    F_x_L.push_back(u_L*(E_L + p_L));
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_x_intercell_HLLC.push_back(F_x_L[ei] + s_minus*(Q_star_L[ei] - *(Q_L[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_L >= 0)
                    {
                        F_x_intercell_HLL = F_x_L;
                    }
                    else
                    {
                        std::vector<double> F_x_R;
                        F_x_R.push_back(rho_u_R);
                        F_x_R.push_back(rho_u_R*u_R + p_R);
                        F_x_R.push_back(rho_u_R*v_R);
                        F_x_R.push_back(rho_u_R*w_R);
                        F_x_R.push_back(u_R*(E_R + p_R));
                        
                        if (s_R <= 0)
                        {
                            F_x_intercell_HLL = F_x_R;
                        }
                        else
                        {
                            std::vector<const double*> Q_R;
                            Q_R.push_back(&rho_R);
                            Q_R.push_back(m_R[0]);
                            Q_R.push_back(m_R[1]);
                            Q_R.push_back(m_R[2]);
                            Q_R.push_back(&E_R);
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_x_intercell_HLL.push_back((s_R*F_x_L[ei] - s_L*F_x_R[ei] +
                                    s_R*s_L*(*(Q_R[ei]) - *(Q_L[ei])))/(s_R - s_L));
                            }
                        }
                    }
                }
                else
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_R = (s_R - u_R)/(s_R - s_star);
                    
                    std::vector<double> Q_star_R;
                    Q_star_R.push_back(Chi_star_R*rho_R);
                    Q_star_R.push_back(Chi_star_R*rho_R*s_star);
                    Q_star_R.push_back(Chi_star_R*rho_v_R);
                    Q_star_R.push_back(Chi_star_R*rho_w_R);
                    Q_star_R.push_back(Chi_star_R*(E_R + (s_star - u_R)*(rho_R*s_star + p_R/(s_R - u_R))));
                    
                    std::vector<const double*> Q_R;
                    Q_R.push_back(&rho_R);
                    Q_R.push_back(m_R[0]);
                    Q_R.push_back(m_R[1]);
                    Q_R.push_back(m_R[2]);
                    Q_R.push_back(&E_R);
                    
                    std::vector<double> F_x_R;
                    F_x_R.push_back(rho_u_R);
                    F_x_R.push_back(rho_u_R*u_R + p_R);
                    F_x_R.push_back(rho_u_R*v_R);
                    F_x_R.push_back(rho_u_R*w_R);
                    F_x_R.push_back(u_R*(E_R + p_R));
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_x_intercell_HLLC.push_back(F_x_R[ei] + s_plus*(Q_star_R[ei] - *(Q_R[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_R <= 0)
                    {
                        F_x_intercell_HLL = F_x_R;
                    }
                    else
                    {
                        std::vector<double> F_x_L;
                        F_x_L.push_back(rho_u_L);
                        F_x_L.push_back(rho_u_L*u_L + p_L);
                        F_x_L.push_back(rho_u_L*v_L);
                        F_x_L.push_back(rho_u_L*w_L);
                        F_x_L.push_back(u_L*(E_L + p_L));
                        
                        if (s_L >= 0)
                        {
                            F_x_intercell_HLL = F_x_L;
                        }
                        else
                        {
                            std::vector<const double*> Q_L;
                            Q_L.push_back(&rho_L);
                            Q_L.push_back(m_L[0]);
                            Q_L.push_back(m_L[1]);
                            Q_L.push_back(m_L[2]);
                            Q_L.push_back(&E_L);
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_x_intercell_HLL.push_back((s_R*F_x_L[ei] - s_L*F_x_R[ei] +
                                    s_R*s_L*(*(Q_R[ei]) - *(Q_L[ei])))/(s_R - s_L));
                            }
                        }
                    }
                }
                
                /*
                 * Calulate the weights beta for hybridization.
                 */
                
                double alpha1, alpha2;
                
                double vel_mag = sqrt(pow(u_R - u_L, 2) + pow(v_R - v_L, 2) + pow(w_R - w_L, 2));
                
                if (vel_mag < 1e-40)
                {
                    alpha1 = 1.0;
                    alpha2 = 0.0;
                }
                else
                {
                    alpha1 = fabs(u_R - u_L)/vel_mag;
                    alpha2 = sqrt(1.0 - alpha1*alpha1);
                }
                
                double beta1 = 0.5 + 0.5*alpha1/(alpha1 + alpha2);
                double beta2 = 1.0 - beta1;
                
                /*
                 * Compute the HLLC-HLL flux.
                 */
                
                *(F_x_intercell[0]) = beta1*F_x_intercell_HLLC[0] + beta2*F_x_intercell_HLL[0];
                *(F_x_intercell[1]) = F_x_intercell_HLLC[1];
                *(F_x_intercell[2]) = beta1*F_x_intercell_HLLC[2] + beta2*F_x_intercell_HLL[2];
                *(F_x_intercell[3]) = beta1*F_x_intercell_HLLC[3] + beta2*F_x_intercell_HLL[3];
                *(F_x_intercell[4]) = F_x_intercell_HLLC[4];
                
                break;
            }
            case Y_DIRECTION:
            {
                std::vector<double*> F_y_intercell = flux_intercell;
                
                std::vector<double> F_y_intercell_HLLC;
                std::vector<double> F_y_intercell_HLL;
                
                const double& rho_B = *density_L;
                const double& rho_T = *density_R;
                
                const double& rho_u_B = *(momentum_L[0]);
                const double& rho_u_T = *(momentum_R[0]);
                
                const double& rho_v_B = *(momentum_L[1]);
                const double& rho_v_T = *(momentum_R[1]);
                
                const double& rho_w_B = *(momentum_L[2]);
                const double& rho_w_T = *(momentum_R[2]);
                
                const std::vector<const double*> m_B = momentum_L;
                const std::vector<const double*> m_T = momentum_R;
                
                const double& u_B = rho_u_B/rho_B;
                const double& u_T = rho_u_T/rho_T;
                
                const double& v_B = rho_v_B/rho_B;
                const double& v_T = rho_v_T/rho_T;
                
                const double& w_B = rho_w_B/rho_B;
                const double& w_T = rho_w_T/rho_T;
                
                const double& E_B = *total_energy_L;
                const double& E_T = *total_energy_R;
                
                const double p_B = d_equation_of_state->getPressure(
                    &rho_B,
                    m_B,
                    &E_B);
                
                const double p_T = d_equation_of_state->getPressure(
                    &rho_T,
                    m_T,
                    &E_T);
                
                const double c_B = d_equation_of_state->getSoundSpeedWithPressure(
                    &rho_B,
                    &p_B);
                
                const double c_T = d_equation_of_state->getSoundSpeedWithPressure(
                    &rho_T,
                    &p_T);
                
                const double v_average = 0.5*(v_B + v_T);
                const double c_average = 0.5*(c_B + c_T);
                
                const double s_B = fmin(v_average - c_average, v_B - c_B);
                const double s_T = fmax(v_average + c_average, v_T + c_T);
                
                const double s_minus = fmin(0.0, s_B);
                const double s_plus  = fmax(0.0, s_T);
                
                const double s_star =
                    (p_T - p_B + rho_B*v_B*(s_B - v_B) - rho_T*v_T*(s_T - v_T))/(rho_B*(s_B - v_B) -
                        rho_T*(s_T - v_T));
                
                if (s_star > 0)
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_B = (s_B - v_B)/(s_B - s_star);
                    
                    std::vector<double> Q_star_B;
                    Q_star_B.push_back(Chi_star_B*rho_B);
                    Q_star_B.push_back(Chi_star_B*rho_u_B);
                    Q_star_B.push_back(Chi_star_B*rho_B*s_star);
                    Q_star_B.push_back(Chi_star_B*rho_w_B);
                    Q_star_B.push_back(Chi_star_B*(E_B + (s_star - v_B)*(rho_B*s_star + p_B/(s_B - v_B))));
                    
                    std::vector<const double*> Q_B;
                    Q_B.push_back(&rho_B);
                    Q_B.push_back(m_B[0]);
                    Q_B.push_back(m_B[1]);
                    Q_B.push_back(m_B[2]);
                    Q_B.push_back(&E_B);
                    
                    std::vector<double> F_y_B;
                    F_y_B.push_back(rho_v_B);
                    F_y_B.push_back(rho_v_B*u_B);
                    F_y_B.push_back(rho_v_B*v_B + p_B);
                    F_y_B.push_back(rho_v_B*w_B);
                    F_y_B.push_back(v_B*(E_B + p_B));
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_y_intercell_HLLC.push_back(F_y_B[ei] + s_minus*(Q_star_B[ei] - *(Q_B[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_B >= 0)
                    {
                        F_y_intercell_HLL = F_y_B;
                    }
                    else
                    {
                        std::vector<double> F_y_T;
                        F_y_T.push_back(rho_v_T);
                        F_y_T.push_back(rho_v_T*u_T);
                        F_y_T.push_back(rho_v_T*v_T + p_T);
                        F_y_T.push_back(rho_v_T*w_T);
                        F_y_T.push_back(v_T*(E_T + p_T));
                        
                        if (s_T <= 0)
                        {
                            F_y_intercell_HLL = F_y_T;
                        }
                        else
                        {
                            std::vector<const double*> Q_T;
                            Q_T.push_back(&rho_T);
                            Q_T.push_back(m_T[0]);
                            Q_T.push_back(m_T[1]);
                            Q_T.push_back(m_T[2]);
                            Q_T.push_back(&E_T);
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_y_intercell_HLL.push_back((s_T*F_y_B[ei] - s_B*F_y_T[ei] +
                                    s_T*s_B*(*(Q_T[ei]) - *(Q_B[ei])))/(s_T - s_B));
                            }
                        }
                    }
                }
                else
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_T = (s_T - v_T)/(s_T - s_star);
                    
                    std::vector<double> Q_star_T;
                    Q_star_T.push_back(Chi_star_T*rho_T);
                    Q_star_T.push_back(Chi_star_T*rho_u_T);
                    Q_star_T.push_back(Chi_star_T*rho_T*s_star);
                    Q_star_T.push_back(Chi_star_T*rho_w_T);
                    Q_star_T.push_back(Chi_star_T*(E_T + (s_star - v_T)*(rho_T*s_star + p_T/(s_T - v_T))));
                    
                    std::vector<const double*> Q_T;
                    Q_T.push_back(&rho_T);
                    Q_T.push_back(m_T[0]);
                    Q_T.push_back(m_T[1]);
                    Q_T.push_back(m_T[2]);
                    Q_T.push_back(&E_T);
                    
                    std::vector<double> F_y_T;
                    F_y_T.push_back(rho_v_T);
                    F_y_T.push_back(rho_v_T*u_T);
                    F_y_T.push_back(rho_v_T*v_T + p_T);
                    F_y_T.push_back(rho_v_T*w_T);
                    F_y_T.push_back(v_T*(E_T + p_T));
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_y_intercell_HLLC.push_back(F_y_T[ei] + s_plus*(Q_star_T[ei] - *(Q_T[ei])));
                    }
                    
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    if (s_T <= 0)
                    {
                        F_y_intercell_HLL = F_y_T;
                    }
                    else
                    {
                        std::vector<double> F_y_B;
                        F_y_B.push_back(rho_v_B);
                        F_y_B.push_back(rho_v_B*u_B);
                        F_y_B.push_back(rho_v_B*v_B + p_B);
                        F_y_B.push_back(rho_v_B*w_B);
                        F_y_B.push_back(v_B*(E_B + p_B));
                        
                        if (s_B >= 0)
                        {
                            F_y_intercell_HLL = F_y_B;
                        }
                        else
                        {
                            std::vector<const double*> Q_B;
                            Q_B.push_back(&rho_B);
                            Q_B.push_back(m_B[0]);
                            Q_B.push_back(m_B[1]);
                            Q_B.push_back(m_B[2]);
                            Q_B.push_back(&E_B);
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_y_intercell_HLL.push_back((s_T*F_y_B[ei] - s_B*F_y_T[ei] +
                                    s_T*s_B*(*(Q_T[ei]) - *(Q_B[ei])))/(s_T - s_B));
                            }
                        }
                    }
                }
                
                /*
                 * Calulate the weights beta for hybridization.
                 */
                
                double alpha1, alpha2;
                
                double vel_mag = sqrt(pow(u_T - u_B, 2) + pow(v_T - v_B, 2) + pow(w_T - w_B, 2));
                
                if (vel_mag < 1e-40)
                {
                    alpha1 = 1.0;
                    alpha2 = 0.0;
                }
                else
                {
                    alpha1 = fabs(v_T - v_B)/vel_mag;
                    alpha2 = sqrt(1.0 - alpha1*alpha1);
                }
                
                double beta1 = 0.5 + 0.5*alpha1/(alpha1 + alpha2);
                double beta2 = 1.0 - beta1;
                
                /*
                 * Compute the HLLC-HLL flux.
                 */
                
                *(F_y_intercell[0]) = beta1*F_y_intercell_HLLC[0] + beta2*F_y_intercell_HLL[0];
                *(F_y_intercell[1]) = beta1*F_y_intercell_HLLC[1] + beta2*F_y_intercell_HLL[1];
                *(F_y_intercell[2]) = F_y_intercell_HLLC[2];
                *(F_y_intercell[3]) = beta1*F_y_intercell_HLLC[3] + beta2*F_y_intercell_HLL[3];
                *(F_y_intercell[4]) = F_y_intercell_HLLC[4];
                
                break;
            }
            case Z_DIRECTION:
            {
                std::vector<double*> F_z_intercell = flux_intercell;
                
                std::vector<double> F_z_intercell_HLLC;
                std::vector<double> F_z_intercell_HLL;
                
                const double& rho_B = *density_L;
                const double& rho_F = *density_R;
                
                const double& rho_u_B = *(momentum_L[0]);
                const double& rho_u_F = *(momentum_R[0]);
                
                const double& rho_v_B = *(momentum_L[1]);
                const double& rho_v_F = *(momentum_R[1]);
                
                const double& rho_w_B = *(momentum_L[2]);
                const double& rho_w_F = *(momentum_R[2]);
                
                const std::vector<const double*> m_B = momentum_L;
                const std::vector<const double*> m_F = momentum_R;
                
                const double& u_B = rho_u_B/rho_B;
                const double& u_F = rho_u_F/rho_F;
                
                const double& v_B = rho_v_B/rho_B;
                const double& v_F = rho_v_F/rho_F;
                
                const double& w_B = rho_w_B/rho_B;
                const double& w_F = rho_w_F/rho_F;
                
                const double& E_B = *total_energy_L;
                const double& E_F = *total_energy_R;
                
                const double p_B = d_equation_of_state->getPressure(
                    &rho_B,
                    m_B,
                    &E_B);
                
                const double p_F = d_equation_of_state->getPressure(
                    &rho_F,
                    m_F,
                    &E_F);
                
                const double c_B = d_equation_of_state->getSoundSpeedWithPressure(
                    &rho_B,
                    &p_B);
                
                const double c_F = d_equation_of_state->getSoundSpeedWithPressure(
                    &rho_F,
                    &p_F);
                
                const double w_average = 0.5*(w_B + w_F);
                const double c_average = 0.5*(c_B + c_F);
                
                const double s_B = fmin(w_average - c_average, w_B - c_B);
                const double s_F = fmax(w_average + c_average, w_F + c_F);
                
                const double s_minus = fmin(0.0, s_B);
                const double s_plus  = fmax(0.0, s_F);
                
                const double s_star =
                    (p_F - p_B + rho_B*w_B*(s_B - w_B) - rho_F*w_F*(s_F - w_F))/(rho_B*(s_B - w_B) -
                        rho_F*(s_F - w_F));
                
                if (s_star > 0)
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_B = (s_B - w_B)/(s_B - s_star);
                    
                    std::vector<double> Q_star_B;
                    Q_star_B.push_back(Chi_star_B*rho_B);
                    Q_star_B.push_back(Chi_star_B*rho_u_B);
                    Q_star_B.push_back(Chi_star_B*rho_v_B);
                    Q_star_B.push_back(Chi_star_B*rho_B*s_star);
                    Q_star_B.push_back(Chi_star_B*(E_B + (s_star - w_B)*(rho_B*s_star + p_B/(s_B - w_B))));
                    
                    std::vector<const double*> Q_B;
                    Q_B.push_back(&rho_B);
                    Q_B.push_back(m_B[0]);
                    Q_B.push_back(m_B[1]);
                    Q_B.push_back(m_B[2]);
                    Q_B.push_back(&E_B);
                    
                    std::vector<double> F_z_B;
                    F_z_B.push_back(rho_w_B);
                    F_z_B.push_back(rho_w_B*u_B);
                    F_z_B.push_back(rho_w_B*v_B);
                    F_z_B.push_back(rho_w_B*w_B + p_B);
                    F_z_B.push_back(w_B*(E_B + p_B));
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_z_intercell_HLLC.push_back(F_z_B[ei] + s_minus*(Q_star_B[ei] - *(Q_B[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_B >= 0)
                    {
                        F_z_intercell_HLL = F_z_B;
                    }
                    else
                    {
                        std::vector<double> F_z_F;
                        F_z_F.push_back(rho_w_F);
                        F_z_F.push_back(rho_w_F*u_F);
                        F_z_F.push_back(rho_w_F*v_F);
                        F_z_F.push_back(rho_w_F*w_F + p_F);
                        F_z_F.push_back(w_F*(E_F + p_F));
                        
                        if (s_F <= 0)
                        {
                            F_z_intercell_HLL = F_z_F;
                        }
                        else
                        {
                            std::vector<const double*> Q_F;
                            Q_F.push_back(&rho_F);
                            Q_F.push_back(m_F[0]);
                            Q_F.push_back(m_F[1]);
                            Q_F.push_back(m_F[2]);
                            Q_F.push_back(&E_F);
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_z_intercell_HLL.push_back((s_F*F_z_B[ei] - s_B*F_z_F[ei] +
                                    s_F*s_B*(*(Q_F[ei]) - *(Q_B[ei])))/(s_F - s_B));
                            }
                        }
                    }
                }
                else
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_F = (s_F - w_F)/(s_F - s_star);
                    
                    std::vector<double> Q_star_F;
                    Q_star_F.push_back(Chi_star_F*rho_F);
                    Q_star_F.push_back(Chi_star_F*rho_u_F);
                    Q_star_F.push_back(Chi_star_F*rho_v_F);
                    Q_star_F.push_back(Chi_star_F*rho_F*s_star);
                    Q_star_F.push_back(Chi_star_F*(E_F + (s_star - w_F)*(rho_F*s_star + p_F/(s_F - w_F))));
                    
                    std::vector<const double*> Q_F;
                    Q_F.push_back(&rho_F);
                    Q_F.push_back(m_F[0]);
                    Q_F.push_back(m_F[1]);
                    Q_F.push_back(m_F[2]);
                    Q_F.push_back(&E_F);
                    
                    std::vector<double> F_z_F;
                    F_z_F.push_back(rho_w_F);
                    F_z_F.push_back(rho_w_F*u_F);
                    F_z_F.push_back(rho_w_F*v_F);
                    F_z_F.push_back(rho_w_F*w_F + p_F);
                    F_z_F.push_back(w_F*(E_F + p_F));
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_z_intercell_HLLC.push_back(F_z_F[ei] + s_plus*(Q_star_F[ei] - *(Q_F[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_F <= 0)
                    {
                        F_z_intercell_HLL = F_z_F;
                    }
                    else
                    {
                        std::vector<double> F_z_B;
                        F_z_B.push_back(rho_w_B);
                        F_z_B.push_back(rho_w_B*u_B);
                        F_z_B.push_back(rho_w_B*v_B);
                        F_z_B.push_back(rho_w_B*w_B + p_B);
                        F_z_B.push_back(w_B*(E_B + p_B));
                        
                        if (s_B >= 0)
                        {
                            F_z_intercell_HLL = F_z_B;
                        }
                        else
                        {
                            std::vector<const double*> Q_B;
                            Q_B.push_back(&rho_B);
                            Q_B.push_back(m_B[0]);
                            Q_B.push_back(m_B[1]);
                            Q_B.push_back(m_B[2]);
                            Q_B.push_back(&E_B);
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_z_intercell_HLL.push_back((s_F*F_z_B[ei] - s_B*F_z_F[ei] +
                                    s_F*s_B*(*(Q_F[ei]) - *(Q_B[ei])))/(s_F - s_B));
                            }
                        }
                    }
                }
                
                /*
                 * Calulate the weights beta for hybridization.
                 */
                
                double alpha1, alpha2;
                
                double vel_mag = sqrt(pow(u_F - u_B, 2) + pow(v_F - v_B, 2) + pow(w_F - w_B, 2));
                
                if (vel_mag < 1e-40)
                {
                    alpha1 = 1.0;
                    alpha2 = 0.0;
                }
                else
                {
                    alpha1 = fabs(w_F - w_B)/vel_mag;
                    alpha2 = sqrt(1.0 - alpha1*alpha1);
                }
                
                double beta1 = 0.5 + 0.5*alpha1/(alpha1 + alpha2);
                double beta2 = 1.0 - beta1;
                
                /*
                 * Compute the HLLC-HLL flux.
                 */
                
                *(F_z_intercell[0]) = beta1*F_z_intercell_HLLC[0] + beta2*F_z_intercell_HLL[0];
                *(F_z_intercell[1]) = beta1*F_z_intercell_HLLC[1] + beta2*F_z_intercell_HLL[1];
                *(F_z_intercell[2]) = beta1*F_z_intercell_HLLC[2] + beta2*F_z_intercell_HLL[2];
                *(F_z_intercell[3]) = F_z_intercell_HLLC[3];
                *(F_z_intercell[4]) = F_z_intercell_HLLC[4];
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "Unknown direction."
                           << std::endl);
            }
        }
    }
}


/*
 * Compute the fluxes and and velocities at the intercell faces
 * for four-equation multi-species flow model by Shyue.
 */
void
RiemannSolverHLLC_HLL::computeIntercellFluxAndVelocityForFourEqnShyue(
    std::vector<double*> flux_intercell,
    std::vector<double*> velocity_intercell,
    const double* const density_L,
    const double* const density_R,
    const std::vector<const double*> momentum_L,
    const std::vector<const double*> momentum_R,
    const double* const total_energy_L,
    const double* const total_energy_R,
    const std::vector<const double*> mass_fraction_L,
    const std::vector<const double*> mass_fraction_R,
    DIRECTION direction)
{
    if (d_dim == tbox::Dimension(1))
    {
#ifdef DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(static_cast<int>(flux_intercell.size()) == d_num_eqn);
        TBOX_ASSERT(static_cast<int>(velocity_intercell.size()) == 1);
        TBOX_ASSERT(static_cast<int>(momentum_L.size()) == 1);
        TBOX_ASSERT(static_cast<int>(momentum_R.size()) == 1);
        TBOX_ASSERT(static_cast<int>(mass_fraction_L.size()) == d_num_species - 1);
        TBOX_ASSERT(static_cast<int>(mass_fraction_R.size()) == d_num_species - 1);
        
        for (int fi = 0; fi < static_cast<int>(flux_intercell.size()); fi++)
        {
            TBOX_ASSERT(flux_intercell[fi]);
        }
        for (int vi = 0; vi < static_cast<int>(velocity_intercell.size()); vi++)
        {
            TBOX_ASSERT(velocity_intercell[vi]);
        }
        TBOX_ASSERT(density_L);
        TBOX_ASSERT(density_R);
        for (int mi = 0; mi < static_cast<int>(momentum_L.size()); mi++)
        {
            TBOX_ASSERT(momentum_L[mi]);
        }
        for (int mi = 0; mi < static_cast<int>(momentum_R.size()); mi++)
        {
            TBOX_ASSERT(momentum_R[mi]);
        }
        TBOX_ASSERT(total_energy_L);
        TBOX_ASSERT(total_energy_R);
        for (int mi = 0; mi < static_cast<int>(mass_fraction_L.size()); mi++)
        {
            TBOX_ASSERT(mass_fraction_L[mi]);
        }
        for (int mi = 0; mi < static_cast<int>(mass_fraction_R.size()); mi++)
        {
            TBOX_ASSERT(mass_fraction_R[mi]);
        }
#endif
        
        switch (direction)
        {
            case X_DIRECTION:
            {
                std::vector<double*> F_x_intercell = flux_intercell;
                
                double& u_intercell = *(velocity_intercell[0]);
                
                const double& rho_L = *density_L;
                const double& rho_R = *density_R;
                
                const double& rho_u_L = *(momentum_L[0]);
                const double& rho_u_R = *(momentum_R[0]);
                
                const std::vector<const double*> m_L = momentum_L;
                const std::vector<const double*> m_R = momentum_R;
                
                const double& u_L = rho_u_L/rho_L;
                const double& u_R = rho_u_R/rho_R;
                
                const double& E_L = *total_energy_L;
                const double& E_R = *total_energy_R;
                
                const std::vector<const double*> Y_L = mass_fraction_L;
                const std::vector<const double*> Y_R = mass_fraction_R;
                
                const double p_L = d_equation_of_state->
                    getPressureWithMassFraction(
                        &rho_L,
                        m_L,
                        &E_L,
                        Y_L);
                
                const double p_R = d_equation_of_state->
                    getPressureWithMassFraction(
                        &rho_R,
                        m_R,
                        &E_R,
                        Y_R);
                
                const double c_L = d_equation_of_state->
                    getSoundSpeedWithMassFractionAndPressure(
                        &rho_L,
                        Y_L,
                        &p_L);
                
                const double c_R = d_equation_of_state->
                    getSoundSpeedWithMassFractionAndPressure(
                        &rho_R,
                        Y_R,
                        &p_R);
                
                const double u_average = 0.5*(u_L + u_R);
                const double c_average = 0.5*(c_L + c_R);
                
                const double s_L = fmin(u_average - c_average, u_L - c_L);
                const double s_R = fmax(u_average + c_average, u_R + c_R);
                
                const double s_minus = fmin(0.0, s_L);
                const double s_plus  = fmax(0.0, s_R);
                
                const double s_star =
                    (p_R - p_L + rho_L*u_L*(s_L - u_L) - rho_R*u_R*(s_R - u_R))/(rho_L*(s_L - u_L) -
                        rho_R*(s_R - u_R));
                
                if (s_star > 0)
                {
                    const double Chi_star_L = (s_L - u_L)/(s_L - s_star);
                    
                    std::vector<double> Q_star_L;
                    Q_star_L.push_back(Chi_star_L*rho_L);
                    Q_star_L.push_back(Chi_star_L*rho_L*s_star);
                    Q_star_L.push_back(Chi_star_L*(E_L + (s_star - u_L)*(rho_L*s_star + p_L/(s_L - u_L))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_L.push_back(Chi_star_L*(*(Y_L[si])));
                    }
                    
                    std::vector<const double*> Q_L;
                    Q_L.push_back(&rho_L);
                    Q_L.push_back(m_L[0]);
                    Q_L.push_back(&E_L);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_L.push_back(Y_L[si]);
                    }
                    
                    std::vector<double> F_x_L;
                    F_x_L.push_back(rho_u_L);
                    F_x_L.push_back(rho_u_L*u_L + p_L);
                    F_x_L.push_back(u_L*(E_L + p_L));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_x_L.push_back(u_L*(*(Y_L[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        *(F_x_intercell[ei]) = F_x_L[ei] + s_minus*(Q_star_L[ei] - *(Q_L[ei]));
                    }
                    
                    u_intercell = u_L + s_minus*((s_L - u_L)/(s_L - s_star) - 1);
                }
                else
                {
                    const double Chi_star_R = (s_R - u_R)/(s_R - s_star);
                    
                    std::vector<double> Q_star_R;
                    Q_star_R.push_back(Chi_star_R*rho_R);
                    Q_star_R.push_back(Chi_star_R*rho_R*s_star);
                    Q_star_R.push_back(Chi_star_R*(E_R + (s_star - u_R)*(rho_R*s_star + p_R/(s_R - u_R))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_R.push_back(Chi_star_R*(*(Y_R[si])));
                    }
                    
                    std::vector<const double*> Q_R;
                    Q_R.push_back(&rho_R);
                    Q_R.push_back(m_R[0]);
                    Q_R.push_back(&E_R);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_R.push_back(Y_R[si]);
                    }
                    
                    std::vector<double> F_x_R;
                    F_x_R.push_back(rho_u_R);
                    F_x_R.push_back(rho_u_R*u_R + p_R);
                    F_x_R.push_back(u_R*(E_R + p_R));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_x_R.push_back(u_R*(*(Y_R[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        *(F_x_intercell[ei]) = F_x_R[ei] + s_plus*(Q_star_R[ei] - *(Q_R[ei]));
                    }
                    
                    u_intercell = u_R + s_plus*((s_R - u_R)/(s_R - s_star) - 1);
                }
                
                break;  
            }
            case Y_DIRECTION:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "There is no y direction for 1D problem."
                           << std::endl);
                break;
            }
            case Z_DIRECTION:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "There is no z direction for 1D problem."
                           << std::endl);
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "Unknown direction."
                           << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(2))
    {
#ifdef DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(static_cast<int>(flux_intercell.size()) == d_num_eqn);
        TBOX_ASSERT(static_cast<int>(velocity_intercell.size()) == 2);
        TBOX_ASSERT(static_cast<int>(momentum_L.size()) == 2);
        TBOX_ASSERT(static_cast<int>(momentum_R.size()) == 2);
        TBOX_ASSERT(static_cast<int>(mass_fraction_L.size()) == d_num_species - 1);
        TBOX_ASSERT(static_cast<int>(mass_fraction_R.size()) == d_num_species - 1);
        
        for (int fi = 0; fi < static_cast<int>(flux_intercell.size()); fi++)
        {
            TBOX_ASSERT(flux_intercell[fi]);
        }
        for (int vi = 0; vi < static_cast<int>(velocity_intercell.size()); vi++)
        {
            TBOX_ASSERT(velocity_intercell[vi]);
        }
        TBOX_ASSERT(density_L);
        TBOX_ASSERT(density_R);
        for (int mi = 0; mi < static_cast<int>(momentum_L.size()); mi++)
        {
            TBOX_ASSERT(momentum_L[mi]);
        }
        for (int mi = 0; mi < static_cast<int>(momentum_R.size()); mi++)
        {
            TBOX_ASSERT(momentum_R[mi]);
        }
        TBOX_ASSERT(total_energy_L);
        TBOX_ASSERT(total_energy_R);
        for (int mi = 0; mi < static_cast<int>(mass_fraction_L.size()); mi++)
        {
            TBOX_ASSERT(mass_fraction_L[mi]);
        }
        for (int mi = 0; mi < static_cast<int>(mass_fraction_R.size()); mi++)
        {
            TBOX_ASSERT(mass_fraction_R[mi]);
        }
#endif
        
        switch (direction)
        {
            case X_DIRECTION:
            {
                std::vector<double*> F_x_intercell = flux_intercell;
                
                std::vector<double> F_x_intercell_HLLC;
                std::vector<double> F_x_intercell_HLL;
                
                double& u_intercell = *(velocity_intercell[0]);
                double& v_intercell = *(velocity_intercell[1]);
                
                const double& rho_L = *density_L;
                const double& rho_R = *density_R;
                
                const double& rho_u_L = *(momentum_L[0]);
                const double& rho_u_R = *(momentum_R[0]);
                
                const double& rho_v_L = *(momentum_L[1]);
                const double& rho_v_R = *(momentum_R[1]);
                
                const std::vector<const double*> m_L = momentum_L;
                const std::vector<const double*> m_R = momentum_R;
                
                const double& u_L = rho_u_L/rho_L;
                const double& u_R = rho_u_R/rho_R;
                
                const double& v_L = rho_v_L/rho_L;
                const double& v_R = rho_v_R/rho_R;
                
                const double& E_L = *total_energy_L;
                const double& E_R = *total_energy_R;
                
                const std::vector<const double*> Y_L = mass_fraction_L;
                const std::vector<const double*> Y_R = mass_fraction_R;
                
                const double p_L = d_equation_of_state->
                    getPressureWithMassFraction(
                        &rho_L,
                        m_L,
                        &E_L,
                        Y_L);
                
                const double p_R = d_equation_of_state->
                    getPressureWithMassFraction(
                        &rho_R,
                        m_R,
                        &E_R,
                        Y_R);
                
                const double c_L = d_equation_of_state->
                    getSoundSpeedWithMassFractionAndPressure(
                        &rho_L,
                        Y_L,
                        &p_L);
                
                const double c_R = d_equation_of_state->
                    getSoundSpeedWithMassFractionAndPressure(
                        &rho_R,
                        Y_R,
                        &p_R);
                
                const double u_average = 0.5*(u_L + u_R);
                const double c_average = 0.5*(c_L + c_R);
                
                const double s_L = fmin(u_average - c_average, u_L - c_L);
                const double s_R = fmax(u_average + c_average, u_R + c_R);
                
                const double s_minus = fmin(0.0, s_L);
                const double s_plus  = fmax(0.0, s_R);
                
                const double s_star =
                    (p_R - p_L + rho_L*u_L*(s_L - u_L) - rho_R*u_R*(s_R - u_R))/(rho_L*(s_L - u_L) -
                        rho_R*(s_R - u_R));
                
                if (s_star > 0)
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_L = (s_L - u_L)/(s_L - s_star);
                    
                    std::vector<double> Q_star_L;
                    Q_star_L.push_back(Chi_star_L*rho_L);
                    Q_star_L.push_back(Chi_star_L*rho_L*s_star);
                    Q_star_L.push_back(Chi_star_L*rho_v_L);
                    Q_star_L.push_back(Chi_star_L*(E_L + (s_star - u_L)*(rho_L*s_star + p_L/(s_L - u_L))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_L.push_back(Chi_star_L*(*(Y_L[si])));
                    }
                    
                    std::vector<const double*> Q_L;
                    Q_L.push_back(&rho_L);
                    Q_L.push_back(m_L[0]);
                    Q_L.push_back(m_L[1]);
                    Q_L.push_back(&E_L);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_L.push_back(Y_L[si]);
                    }
                    
                    std::vector<double> F_x_L;
                    F_x_L.push_back(rho_u_L);
                    F_x_L.push_back(rho_u_L*u_L + p_L);
                    F_x_L.push_back(rho_u_L*v_L);
                    F_x_L.push_back(u_L*(E_L + p_L));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_x_L.push_back(u_L*(*(Y_L[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_x_intercell_HLLC.push_back(F_x_L[ei] + s_minus*(Q_star_L[ei] - *(Q_L[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_L >= 0)
                    {
                        F_x_intercell_HLL = F_x_L;
                    }
                    else
                    {
                        std::vector<double> F_x_R;
                        F_x_R.push_back(rho_u_R);
                        F_x_R.push_back(rho_u_R*u_R + p_R);
                        F_x_R.push_back(rho_u_R*v_R);
                        F_x_R.push_back(u_R*(E_R + p_R));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_x_R.push_back(u_R*(*(Y_R[si])));
                        }
                        
                        if (s_R <= 0)
                        {
                            F_x_intercell_HLL = F_x_R;
                        }
                        else
                        {
                            std::vector<const double*> Q_R;
                            Q_R.push_back(&rho_R);
                            Q_R.push_back(m_R[0]);
                            Q_R.push_back(m_R[1]);
                            Q_R.push_back(&E_R);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_R.push_back(Y_R[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_x_intercell_HLL.push_back((s_R*F_x_L[ei] - s_L*F_x_R[ei] +
                                    s_R*s_L*(*(Q_R[ei]) - *(Q_L[ei])))/(s_R - s_L));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_L + s_minus*((s_L - u_L)/(s_L - s_star) - 1);
                    v_intercell = v_L;
                }
                else
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_R = (s_R - u_R)/(s_R - s_star);
                    
                    std::vector<double> Q_star_R;
                    Q_star_R.push_back(Chi_star_R*rho_R);
                    Q_star_R.push_back(Chi_star_R*rho_R*s_star);
                    Q_star_R.push_back(Chi_star_R*rho_v_R);
                    Q_star_R.push_back(Chi_star_R*(E_R + (s_star - u_R)*(rho_R*s_star + p_R/(s_R - u_R))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_R.push_back(Chi_star_R*(*(Y_R[si])));
                    }
                    
                    std::vector<const double*> Q_R;
                    Q_R.push_back(&rho_R);
                    Q_R.push_back(m_R[0]);
                    Q_R.push_back(m_R[1]);
                    Q_R.push_back(&E_R);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_R.push_back(Y_R[si]);
                    }
                    
                    std::vector<double> F_x_R;
                    F_x_R.push_back(rho_u_R);
                    F_x_R.push_back(rho_u_R*u_R + p_R);
                    F_x_R.push_back(rho_u_R*v_R);
                    F_x_R.push_back(u_R*(E_R + p_R));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_x_R.push_back(u_R*(*(Y_R[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_x_intercell_HLLC.push_back(F_x_R[ei] + s_plus*(Q_star_R[ei] - *(Q_R[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_R <= 0)
                    {
                        F_x_intercell_HLL = F_x_R;
                    }
                    else
                    {
                        std::vector<double> F_x_L;
                        F_x_L.push_back(rho_u_L);
                        F_x_L.push_back(rho_u_L*u_L + p_L);
                        F_x_L.push_back(rho_u_L*v_L);
                        F_x_L.push_back(u_L*(E_L + p_L));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_x_L.push_back(u_L*(*(Y_L[si])));
                        }
                        
                        if (s_L >= 0)
                        {
                            F_x_intercell_HLL = F_x_L;
                        }
                        else
                        {
                            std::vector<const double*> Q_L;
                            Q_L.push_back(&rho_L);
                            Q_L.push_back(m_L[0]);
                            Q_L.push_back(m_L[1]);
                            Q_L.push_back(&E_L);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_L.push_back(Y_L[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_x_intercell_HLL.push_back((s_R*F_x_L[ei] - s_L*F_x_R[ei] +
                                    s_R*s_L*(*(Q_R[ei]) - *(Q_L[ei])))/(s_R - s_L));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_R + s_plus*((s_R - u_R)/(s_R - s_star) - 1);
                    v_intercell = v_R;
                }
                
                /*
                 * Calulate the weights beta for hybridization.
                 */
                
                double alpha1, alpha2;
                
                double vel_mag = sqrt(pow(u_R - u_L, 2) + pow(v_R - v_L, 2));
                
                if (vel_mag < 1e-40)
                {
                    alpha1 = 1.0;
                    alpha2 = 0.0;
                }
                else
                {
                    alpha1 = fabs(u_R - u_L)/vel_mag;
                    alpha2 = sqrt(1.0 - alpha1*alpha1);
                }
                
                double beta1 = 0.5 + 0.5*alpha1/(alpha1 + alpha2);
                double beta2 = 1.0 - beta1;
                
                /*
                 * Compute the HLLC-HLL flux.
                 */
                
                *(F_x_intercell[0]) = beta1*F_x_intercell_HLLC[0] + beta2*F_x_intercell_HLL[0];
                *(F_x_intercell[1]) = F_x_intercell_HLLC[1];
                *(F_x_intercell[2]) = beta1*F_x_intercell_HLLC[2] + beta2*F_x_intercell_HLL[2];
                *(F_x_intercell[3]) = F_x_intercell_HLLC[3];
                for (int si = 0; si < d_num_species - 1; si++)
                {
                    *(F_x_intercell[4 + si]) = beta1*F_x_intercell_HLLC[4 + si] + beta2*F_x_intercell_HLL[4 + si];
                    // *(F_x_intercell[4 + si]) = F_x_intercell_HLLC[4 + si];
                }
                
                break;
            }
            case Y_DIRECTION:
            {
                std::vector<double*> F_y_intercell = flux_intercell;
                
                std::vector<double> F_y_intercell_HLLC;
                std::vector<double> F_y_intercell_HLL;
                
                double& u_intercell = *(velocity_intercell[0]);
                double& v_intercell = *(velocity_intercell[1]);
                
                const double& rho_B = *density_L;
                const double& rho_T = *density_R;
                
                const double& rho_u_B = *(momentum_L[0]);
                const double& rho_u_T = *(momentum_R[0]);
                
                const double& rho_v_B = *(momentum_L[1]);
                const double& rho_v_T = *(momentum_R[1]);
                
                const std::vector<const double*> m_B = momentum_L;
                const std::vector<const double*> m_T = momentum_R;
                
                const double& u_B = rho_u_B/rho_B;
                const double& u_T = rho_u_T/rho_T;
                
                const double& v_B = rho_v_B/rho_B;
                const double& v_T = rho_v_T/rho_T;
                
                const double& E_B = *total_energy_L;
                const double& E_T = *total_energy_R;
                
                const std::vector<const double*> Y_B = mass_fraction_L;
                const std::vector<const double*> Y_T = mass_fraction_R;
                
                const double p_B = d_equation_of_state->
                    getPressureWithMassFraction(
                        &rho_B,
                        m_B,
                        &E_B,
                        Y_B);
                
                const double p_T = d_equation_of_state->
                    getPressureWithMassFraction(
                        &rho_T,
                        m_T,
                        &E_T,
                        Y_T);
                
                const double c_B = d_equation_of_state->
                    getSoundSpeedWithMassFractionAndPressure(
                        &rho_B,
                        Y_B,
                        &p_B);
                
                const double c_T = d_equation_of_state->
                    getSoundSpeedWithMassFractionAndPressure(
                        &rho_T,
                        Y_T,
                        &p_T);
                
                const double v_average = 0.5*(v_B + v_T);
                const double c_average = 0.5*(c_B + c_T);
                
                const double s_B = fmin(v_average - c_average, v_B - c_B);
                const double s_T = fmax(v_average + c_average, v_T + c_T);
                
                const double s_minus = fmin(0.0, s_B);
                const double s_plus  = fmax(0.0, s_T);
                
                const double s_star =
                    (p_T - p_B + rho_B*v_B*(s_B - v_B) - rho_T*v_T*(s_T - v_T))/(rho_B*(s_B - v_B) -
                        rho_T*(s_T - v_T));
                
                if (s_star > 0)
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_B = (s_B - v_B)/(s_B - s_star);    
                    
                    std::vector<double> Q_star_B;
                    Q_star_B.push_back(Chi_star_B*rho_B);
                    Q_star_B.push_back(Chi_star_B*rho_u_B);
                    Q_star_B.push_back(Chi_star_B*rho_B*s_star);
                    Q_star_B.push_back(Chi_star_B*(E_B + (s_star - v_B)*(rho_B*s_star + p_B/(s_B - v_B))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_B.push_back(Chi_star_B*(*(Y_B[si])));
                    }
                    
                    std::vector<const double*> Q_B;
                    Q_B.push_back(&rho_B);
                    Q_B.push_back(m_B[0]);
                    Q_B.push_back(m_B[1]);
                    Q_B.push_back(&E_B);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_B.push_back(Y_B[si]);
                    }
                    
                    std::vector<double> F_y_B;
                    F_y_B.push_back(rho_v_B);
                    F_y_B.push_back(rho_v_B*u_B);
                    F_y_B.push_back(rho_v_B*v_B + p_B);
                    F_y_B.push_back(v_B*(E_B + p_B));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_y_B.push_back(v_B*(*(Y_B[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_y_intercell_HLLC.push_back(F_y_B[ei] + s_minus*(Q_star_B[ei] - *(Q_B[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_B >= 0)
                    {
                        F_y_intercell_HLL = F_y_B;
                    }
                    else
                    {
                        std::vector<double> F_y_T;
                        F_y_T.push_back(rho_v_T);
                        F_y_T.push_back(rho_v_T*u_T);
                        F_y_T.push_back(rho_v_T*v_T + p_T);
                        F_y_T.push_back(v_T*(E_T + p_T));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_y_T.push_back(v_T*(*(Y_T[si])));
                        }
                        
                        if (s_T <= 0)
                        {
                            F_y_intercell_HLL = F_y_T;
                        }
                        else
                        {
                            std::vector<const double*> Q_T;
                            Q_T.push_back(&rho_T);
                            Q_T.push_back(m_T[0]);
                            Q_T.push_back(m_T[1]);
                            Q_T.push_back(&E_T);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_T.push_back(Y_T[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_y_intercell_HLL.push_back((s_T*F_y_B[ei] - s_B*F_y_T[ei] +
                                    s_T*s_B*(*(Q_T[ei]) - *(Q_B[ei])))/(s_T - s_B));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_B;
                    v_intercell = v_B + s_minus*((s_B - v_B)/(s_B - s_star) - 1);
                }
                else
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_T = (s_T - v_T)/(s_T - s_star); 
                    
                    std::vector<double> Q_star_T;
                    Q_star_T.push_back(Chi_star_T*rho_T);
                    Q_star_T.push_back(Chi_star_T*rho_u_T);
                    Q_star_T.push_back(Chi_star_T*rho_T*s_star);
                    Q_star_T.push_back(Chi_star_T*(E_T + (s_star - v_T)*(rho_T*s_star + p_T/(s_T - v_T))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_T.push_back(Chi_star_T*(*(Y_T[si])));
                    }
                    
                    std::vector<const double*> Q_T;
                    Q_T.push_back(&rho_T);
                    Q_T.push_back(m_T[0]);
                    Q_T.push_back(m_T[1]);
                    Q_T.push_back(&E_T);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_T.push_back(Y_T[si]);
                    }
                    
                    std::vector<double> F_y_T;
                    F_y_T.push_back(rho_v_T);
                    F_y_T.push_back(rho_v_T*u_T);
                    F_y_T.push_back(rho_v_T*v_T + p_T);
                    F_y_T.push_back(v_T*(E_T + p_T));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_y_T.push_back(v_T*(*(Y_T[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_y_intercell_HLLC.push_back(F_y_T[ei] + s_plus*(Q_star_T[ei] - *(Q_T[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_T <= 0)
                    {
                        F_y_intercell_HLL = F_y_T;
                    }
                    else
                    {
                        std::vector<double> F_y_B;
                        F_y_B.push_back(rho_v_B);
                        F_y_B.push_back(rho_v_B*u_B);
                        F_y_B.push_back(rho_v_B*v_B + p_B);
                        F_y_B.push_back(v_B*(E_B + p_B));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_y_B.push_back(v_B*(*(Y_B[si])));
                        }
                        
                        if (s_B >= 0)
                        {
                            F_y_intercell_HLL = F_y_B;
                        }
                        else
                        {
                            std::vector<const double*> Q_B;
                            Q_B.push_back(&rho_B);
                            Q_B.push_back(m_B[0]);
                            Q_B.push_back(m_B[1]);
                            Q_B.push_back(&E_B);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_B.push_back(Y_B[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_y_intercell_HLL.push_back((s_T*F_y_B[ei] - s_B*F_y_T[ei] +
                                    s_T*s_B*(*(Q_T[ei]) - *(Q_B[ei])))/(s_T - s_B));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_T;
                    v_intercell = v_T + s_plus*((s_T - v_T)/(s_T - s_star) - 1);
                }
                
                /*
                 * Calulate the weights beta for hybridization.
                 */
                
                double alpha1, alpha2;
                
                double vel_mag = sqrt(pow(u_T - u_B, 2) + pow(v_T - v_B, 2));
                
                if (vel_mag < 1e-40)
                {
                    alpha1 = 1.0;
                    alpha2 = 0.0;
                }
                else
                {
                    alpha1 = fabs(v_T - v_B)/vel_mag;
                    alpha2 = sqrt(1.0 - alpha1*alpha1);
                }
                
                double beta1 = 0.5 + 0.5*alpha1/(alpha1 + alpha2);
                double beta2 = 1.0 - beta1;
                
                /*
                 * Compute the HLLC-HLL flux.
                 */
                
                *(F_y_intercell[0]) = beta1*F_y_intercell_HLLC[0] + beta2*F_y_intercell_HLL[0];
                *(F_y_intercell[1]) = beta1*F_y_intercell_HLLC[1] + beta2*F_y_intercell_HLL[1];
                *(F_y_intercell[2]) = F_y_intercell_HLLC[2];
                *(F_y_intercell[3]) = F_y_intercell_HLLC[3];
                for (int si = 0; si < d_num_species - 1; si++)
                {
                    *(F_y_intercell[4 + si]) = beta1*F_y_intercell_HLLC[4 + si] + beta2*F_y_intercell_HLL[4 + si];
                    // *(F_y_intercell[4 + si]) = F_y_intercell_HLLC[4 + si];
                }
                
                break;
            }
            case Z_DIRECTION:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "There is no z direction for 1D problem."
                           << std::endl);
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "Unknown direction."
                           << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(3))
    {
#ifdef DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(static_cast<int>(flux_intercell.size()) == d_num_eqn);
        TBOX_ASSERT(static_cast<int>(velocity_intercell.size()) == 3);
        TBOX_ASSERT(static_cast<int>(momentum_L.size()) == 3);
        TBOX_ASSERT(static_cast<int>(momentum_R.size()) == 3);
        TBOX_ASSERT(static_cast<int>(mass_fraction_L.size()) == d_num_species - 1);
        TBOX_ASSERT(static_cast<int>(mass_fraction_R.size()) == d_num_species - 1);
        
        for (int fi = 0; fi < static_cast<int>(flux_intercell.size()); fi++)
        {
            TBOX_ASSERT(flux_intercell[fi]);
        }
        for (int vi = 0; vi < static_cast<int>(velocity_intercell.size()); vi++)
        {
            TBOX_ASSERT(velocity_intercell[vi]);
        }
        TBOX_ASSERT(density_L);
        TBOX_ASSERT(density_R);
        for (int mi = 0; mi < static_cast<int>(momentum_L.size()); mi++)
        {
            TBOX_ASSERT(momentum_L[mi]);
        }
        for (int mi = 0; mi < static_cast<int>(momentum_R.size()); mi++)
        {
            TBOX_ASSERT(momentum_R[mi]);
        }
        TBOX_ASSERT(total_energy_L);
        TBOX_ASSERT(total_energy_R);
        for (int mi = 0; mi < static_cast<int>(mass_fraction_L.size()); mi++)
        {
            TBOX_ASSERT(mass_fraction_L[mi]);
        }
        for (int mi = 0; mi < static_cast<int>(mass_fraction_R.size()); mi++)
        {
            TBOX_ASSERT(mass_fraction_R[mi]);
        }
#endif
        
        switch (direction)
        {
            case X_DIRECTION:
            {
                std::vector<double*> F_x_intercell = flux_intercell;
                
                std::vector<double> F_x_intercell_HLLC;
                std::vector<double> F_x_intercell_HLL;
                
                double& u_intercell = *(velocity_intercell[0]);
                double& v_intercell = *(velocity_intercell[1]);
                double& w_intercell = *(velocity_intercell[2]);
                
                const double& rho_L = *density_L;
                const double& rho_R = *density_R;
                
                const double& rho_u_L = *(momentum_L[0]);
                const double& rho_u_R = *(momentum_R[0]);
                
                const double& rho_v_L = *(momentum_L[1]);
                const double& rho_v_R = *(momentum_R[1]);
                
                const double& rho_w_L = *(momentum_L[2]);
                const double& rho_w_R = *(momentum_R[2]);
                
                const std::vector<const double*> m_L = momentum_L;
                const std::vector<const double*> m_R = momentum_R;
                
                const double& u_L = rho_u_L/rho_L;
                const double& u_R = rho_u_R/rho_R;
                
                const double& v_L = rho_v_L/rho_L;
                const double& v_R = rho_v_R/rho_R;
                
                const double& w_L = rho_w_L/rho_L;
                const double& w_R = rho_w_R/rho_R;
                
                const double& E_L = *total_energy_L;
                const double& E_R = *total_energy_R;
                
                const std::vector<const double*> Y_L = mass_fraction_L;
                const std::vector<const double*> Y_R = mass_fraction_R;
                
                const double p_L = d_equation_of_state->
                    getPressureWithMassFraction(
                        &rho_L,
                        m_L,
                        &E_L,
                        Y_L);
                
                const double p_R = d_equation_of_state->
                    getPressureWithMassFraction(
                        &rho_R,
                        m_R,
                        &E_R,
                        Y_R);
                
                const double c_L = d_equation_of_state->
                    getSoundSpeedWithMassFractionAndPressure(
                        &rho_L,
                        Y_L,
                        &p_L);
                
                const double c_R = d_equation_of_state->
                    getSoundSpeedWithMassFractionAndPressure(
                        &rho_R,
                        Y_R,
                        &p_R);
                
                const double u_average = 0.5*(u_L + u_R);
                const double c_average = 0.5*(c_L + c_R);
                
                const double s_L = fmin(u_average - c_average, u_L - c_L);
                const double s_R = fmax(u_average + c_average, u_R + c_R);
                
                const double s_minus = fmin(0.0, s_L);
                const double s_plus  = fmax(0.0, s_R);
                
                const double s_star =
                    (p_R - p_L + rho_L*u_L*(s_L - u_L) - rho_R*u_R*(s_R - u_R))/(rho_L*(s_L - u_L) -
                        rho_R*(s_R - u_R));
                
                if (s_star > 0)
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_L = (s_L - u_L)/(s_L - s_star);
                    
                    std::vector<double> Q_star_L;
                    Q_star_L.push_back(Chi_star_L*rho_L);
                    Q_star_L.push_back(Chi_star_L*rho_L*s_star);
                    Q_star_L.push_back(Chi_star_L*rho_v_L);
                    Q_star_L.push_back(Chi_star_L*rho_w_L);
                    Q_star_L.push_back(Chi_star_L*(E_L + (s_star - u_L)*(rho_L*s_star + p_L/(s_L - u_L))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_L.push_back(Chi_star_L*(*(Y_L[si])));
                    }
                    
                    std::vector<const double*> Q_L;
                    Q_L.push_back(&rho_L);
                    Q_L.push_back(m_L[0]);
                    Q_L.push_back(m_L[1]);
                    Q_L.push_back(m_L[2]);
                    Q_L.push_back(&E_L);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_L.push_back(Y_L[si]);
                    }
                    
                    std::vector<double> F_x_L;
                    F_x_L.push_back(rho_u_L);
                    F_x_L.push_back(rho_u_L*u_L + p_L);
                    F_x_L.push_back(rho_u_L*v_L);
                    F_x_L.push_back(rho_u_L*w_L);
                    F_x_L.push_back(u_L*(E_L + p_L));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_x_L.push_back(u_L*(*(Y_L[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_x_intercell_HLLC.push_back(F_x_L[ei] + s_minus*(Q_star_L[ei] - *(Q_L[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_L >= 0)
                    {
                        F_x_intercell_HLL = F_x_L;
                    }
                    else
                    {
                        std::vector<double> F_x_R;
                        F_x_R.push_back(rho_u_R);
                        F_x_R.push_back(rho_u_R*u_R + p_R);
                        F_x_R.push_back(rho_u_R*v_R);
                        F_x_R.push_back(rho_u_R*w_R);
                        F_x_R.push_back(u_R*(E_R + p_R));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_x_R.push_back(u_R*(*(Y_R[si])));
                        }
                        
                        if (s_R <= 0)
                        {
                            F_x_intercell_HLL = F_x_R;
                        }
                        else
                        {
                            std::vector<const double*> Q_R;
                            Q_R.push_back(&rho_R);
                            Q_R.push_back(m_R[0]);
                            Q_R.push_back(m_R[1]);
                            Q_R.push_back(m_R[2]);
                            Q_R.push_back(&E_R);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_R.push_back(Y_R[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_x_intercell_HLL.push_back((s_R*F_x_L[ei] - s_L*F_x_R[ei] +
                                    s_R*s_L*(*(Q_R[ei]) - *(Q_L[ei])))/(s_R - s_L));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_L + s_minus*((s_L - u_L)/(s_L - s_star) - 1);
                    v_intercell = v_L;
                    w_intercell = w_L;
                }
                else
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_R = (s_R - u_R)/(s_R - s_star);
                    
                    std::vector<double> Q_star_R;
                    Q_star_R.push_back(Chi_star_R*rho_R);
                    Q_star_R.push_back(Chi_star_R*rho_R*s_star);
                    Q_star_R.push_back(Chi_star_R*rho_v_R);
                    Q_star_R.push_back(Chi_star_R*rho_w_R);
                    Q_star_R.push_back(Chi_star_R*(E_R + (s_star - u_R)*(rho_R*s_star + p_R/(s_R - u_R))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_R.push_back(Chi_star_R*(*(Y_R[si])));
                    }
                    
                    std::vector<const double*> Q_R;
                    Q_R.push_back(&rho_R);
                    Q_R.push_back(m_R[0]);
                    Q_R.push_back(m_R[1]);
                    Q_R.push_back(m_R[2]);
                    Q_R.push_back(&E_R);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_R.push_back(Y_R[si]);
                    }
                    
                    std::vector<double> F_x_R;
                    F_x_R.push_back(rho_u_R);
                    F_x_R.push_back(rho_u_R*u_R + p_R);
                    F_x_R.push_back(rho_u_R*v_R);
                    F_x_R.push_back(rho_u_R*w_R);
                    F_x_R.push_back(u_R*(E_R + p_R));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_x_R.push_back(u_R*(*(Y_R[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_x_intercell_HLLC.push_back(F_x_R[ei] + s_plus*(Q_star_R[ei] - *(Q_R[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_R <= 0)
                    {
                        F_x_intercell_HLL = F_x_R;
                    }
                    else
                    {
                        std::vector<double> F_x_L;
                        F_x_L.push_back(rho_u_L);
                        F_x_L.push_back(rho_u_L*u_L + p_L);
                        F_x_L.push_back(rho_u_L*v_L);
                        F_x_L.push_back(rho_u_L*w_L);
                        F_x_L.push_back(u_L*(E_L + p_L));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_x_L.push_back(u_L*(*(Y_L[si])));
                        }
                        
                        if (s_L >= 0)
                        {
                            F_x_intercell_HLL = F_x_L;
                        }
                        else
                        {
                            std::vector<const double*> Q_L;
                            Q_L.push_back(&rho_L);
                            Q_L.push_back(m_L[0]);
                            Q_L.push_back(m_L[1]);
                            Q_L.push_back(m_L[2]);
                            Q_L.push_back(&E_L);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_L.push_back(Y_L[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_x_intercell_HLL.push_back((s_R*F_x_L[ei] - s_L*F_x_R[ei] +
                                    s_R*s_L*(*(Q_R[ei]) - *(Q_L[ei])))/(s_R - s_L));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_R + s_plus*((s_R - u_R)/(s_R - s_star) - 1);
                    v_intercell = v_R;
                    w_intercell = w_R;
                }
                
                /*
                 * Calulate the weights beta for hybridization.
                 */
                
                double alpha1, alpha2;
                
                double vel_mag = sqrt(pow(u_R - u_L, 2) + pow(v_R - v_L, 2) + pow(w_R - w_L, 2));
                
                if (vel_mag < 1e-40)
                {
                    alpha1 = 1.0;
                    alpha2 = 0.0;
                }
                else
                {
                    alpha1 = fabs(u_R - u_L)/vel_mag;
                    alpha2 = sqrt(1.0 - alpha1*alpha1);
                }
                
                double beta1 = 0.5 + 0.5*alpha1/(alpha1 + alpha2);
                double beta2 = 1.0 - beta1;
                
                /*
                 * Compute the HLLC-HLL flux.
                 */
                
                *(F_x_intercell[0]) = beta1*F_x_intercell_HLLC[0] + beta2*F_x_intercell_HLL[0];
                *(F_x_intercell[1]) = F_x_intercell_HLLC[1];
                *(F_x_intercell[2]) = beta1*F_x_intercell_HLLC[2] + beta2*F_x_intercell_HLL[2];
                *(F_x_intercell[3]) = beta1*F_x_intercell_HLLC[3] + beta2*F_x_intercell_HLL[3];
                *(F_x_intercell[4]) = F_x_intercell_HLLC[4];
                for (int si = 0; si < d_num_species - 1; si++)
                {
                    *(F_x_intercell[5 + si]) = beta1*F_x_intercell_HLLC[5 + si] + beta2*F_x_intercell_HLL[5 + si];
                    // *(F_x_intercell[5 + si]) = F_x_intercell_HLLC[5 + si];
                }
                
                break;
            }
            case Y_DIRECTION:
            {
                std::vector<double*> F_y_intercell = flux_intercell;
                
                std::vector<double> F_y_intercell_HLLC;
                std::vector<double> F_y_intercell_HLL;
                
                double& u_intercell = *(velocity_intercell[0]);
                double& v_intercell = *(velocity_intercell[1]);
                double& w_intercell = *(velocity_intercell[2]);
                
                const double& rho_B = *density_L;
                const double& rho_T = *density_R;
                
                const double& rho_u_B = *(momentum_L[0]);
                const double& rho_u_T = *(momentum_R[0]);
                
                const double& rho_v_B = *(momentum_L[1]);
                const double& rho_v_T = *(momentum_R[1]);
                
                const double& rho_w_B = *(momentum_L[2]);
                const double& rho_w_T = *(momentum_R[2]);
                
                const std::vector<const double*> m_B = momentum_L;
                const std::vector<const double*> m_T = momentum_R;
                
                const double& u_B = rho_u_B/rho_B;
                const double& u_T = rho_u_T/rho_T;
                
                const double& v_B = rho_v_B/rho_B;
                const double& v_T = rho_v_T/rho_T;
                
                const double& w_B = rho_w_B/rho_B;
                const double& w_T = rho_w_T/rho_T;
                
                const double& E_B = *total_energy_L;
                const double& E_T = *total_energy_R;
                
                const std::vector<const double*> Y_B = mass_fraction_L;
                const std::vector<const double*> Y_T = mass_fraction_R;
                
                const double p_B = d_equation_of_state->
                    getPressureWithMassFraction(
                        &rho_B,
                        m_B,
                        &E_B,
                        Y_B);
                
                const double p_T = d_equation_of_state->
                    getPressureWithMassFraction(
                        &rho_T,
                        m_T,
                        &E_T,
                        Y_T);
                
                const double c_B = d_equation_of_state->
                    getSoundSpeedWithMassFractionAndPressure(
                        &rho_B,
                        Y_B,
                        &p_B);
                
                const double c_T = d_equation_of_state->
                    getSoundSpeedWithMassFractionAndPressure(
                        &rho_T,
                        Y_T,
                        &p_T);
                
                const double v_average = 0.5*(v_B + v_T);
                const double c_average = 0.5*(c_B + c_T);
                
                const double s_B = fmin(v_average - c_average, v_B - c_B);
                const double s_T = fmax(v_average + c_average, v_T + c_T);
                
                const double s_minus = fmin(0.0, s_B);
                const double s_plus  = fmax(0.0, s_T);
                
                const double s_star =
                    (p_T - p_B + rho_B*v_B*(s_B - v_B) - rho_T*v_T*(s_T - v_T))/(rho_B*(s_B - v_B) -
                        rho_T*(s_T - v_T));
                
                if (s_star > 0)
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_B = (s_B - v_B)/(s_B - s_star);    
                    
                    std::vector<double> Q_star_B;
                    Q_star_B.push_back(Chi_star_B*rho_B);
                    Q_star_B.push_back(Chi_star_B*rho_u_B);
                    Q_star_B.push_back(Chi_star_B*rho_B*s_star);
                    Q_star_B.push_back(Chi_star_B*rho_w_B);
                    Q_star_B.push_back(Chi_star_B*(E_B + (s_star - v_B)*(rho_B*s_star + p_B/(s_B - v_B))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_B.push_back(Chi_star_B*(*(Y_B[si])));
                    }
                    
                    std::vector<const double*> Q_B;
                    Q_B.push_back(&rho_B);
                    Q_B.push_back(m_B[0]);
                    Q_B.push_back(m_B[1]);
                    Q_B.push_back(m_B[2]);
                    Q_B.push_back(&E_B);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_B.push_back(Y_B[si]);
                    }
                    
                    std::vector<double> F_y_B;
                    F_y_B.push_back(rho_v_B);
                    F_y_B.push_back(rho_v_B*u_B);
                    F_y_B.push_back(rho_v_B*v_B + p_B);
                    F_y_B.push_back(rho_v_B*w_B);
                    F_y_B.push_back(v_B*(E_B + p_B));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_y_B.push_back(v_B*(*(Y_B[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_y_intercell_HLLC.push_back(F_y_B[ei] + s_minus*(Q_star_B[ei] - *(Q_B[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_B >= 0)
                    {
                        F_y_intercell_HLL = F_y_B;
                    }
                    else
                    {
                        std::vector<double> F_y_T;
                        F_y_T.push_back(rho_v_T);
                        F_y_T.push_back(rho_v_T*u_T);
                        F_y_T.push_back(rho_v_T*v_T + p_T);
                        F_y_T.push_back(rho_v_T*w_T);
                        F_y_T.push_back(v_T*(E_T + p_T));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_y_T.push_back(v_T*(*(Y_T[si])));
                        }
                        
                        if (s_T <= 0)
                        {
                            F_y_intercell_HLL = F_y_T;
                        }
                        else
                        {
                            std::vector<const double*> Q_T;
                            Q_T.push_back(&rho_T);
                            Q_T.push_back(m_T[0]);
                            Q_T.push_back(m_T[1]);
                            Q_T.push_back(m_T[2]);
                            Q_T.push_back(&E_T);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_T.push_back(Y_T[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_y_intercell_HLL.push_back((s_T*F_y_B[ei] - s_B*F_y_T[ei] +
                                    s_T*s_B*(*(Q_T[ei]) - *(Q_B[ei])))/(s_T - s_B));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_B;
                    v_intercell = v_B + s_minus*((s_B - v_B)/(s_B - s_star) - 1);
                    w_intercell = w_B;
                }
                else
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_T = (s_T - v_T)/(s_T - s_star); 
                    
                    std::vector<double> Q_star_T;
                    Q_star_T.push_back(Chi_star_T*rho_T);
                    Q_star_T.push_back(Chi_star_T*rho_u_T);
                    Q_star_T.push_back(Chi_star_T*rho_T*s_star);
                    Q_star_T.push_back(Chi_star_T*rho_w_T);
                    Q_star_T.push_back(Chi_star_T*(E_T + (s_star - v_T)*(rho_T*s_star + p_T/(s_T - v_T))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_T.push_back(Chi_star_T*(*(Y_T[si])));
                    }
                    
                    std::vector<const double*> Q_T;
                    Q_T.push_back(&rho_T);
                    Q_T.push_back(m_T[0]);
                    Q_T.push_back(m_T[1]);
                    Q_T.push_back(m_T[2]);
                    Q_T.push_back(&E_T);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_T.push_back(Y_T[si]);
                    }
                    
                    std::vector<double> F_y_T;
                    F_y_T.push_back(rho_v_T);
                    F_y_T.push_back(rho_v_T*u_T);
                    F_y_T.push_back(rho_v_T*v_T + p_T);
                    F_y_T.push_back(rho_v_T*w_T);
                    F_y_T.push_back(v_T*(E_T + p_T));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_y_T.push_back(v_T*(*(Y_T[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_y_intercell_HLLC.push_back(F_y_T[ei] + s_plus*(Q_star_T[ei] - *(Q_T[ei])));
                    }
                    
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    if (s_T <= 0)
                    {
                        F_y_intercell_HLL = F_y_T;
                    }
                    else
                    {
                        std::vector<double> F_y_B;
                        F_y_B.push_back(rho_v_B);
                        F_y_B.push_back(rho_v_B*u_B);
                        F_y_B.push_back(rho_v_B*v_B + p_B);
                        F_y_B.push_back(rho_v_B*w_B);
                        F_y_B.push_back(v_B*(E_B + p_B));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_y_B.push_back(v_B*(*(Y_B[si])));
                        }
                        
                        if (s_B >= 0)
                        {
                            F_y_intercell_HLL = F_y_B;
                        }
                        else
                        {
                            std::vector<const double*> Q_B;
                            Q_B.push_back(&rho_B);
                            Q_B.push_back(m_B[0]);
                            Q_B.push_back(m_B[1]);
                            Q_B.push_back(m_B[2]);
                            Q_B.push_back(&E_B);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_B.push_back(Y_B[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_y_intercell_HLL.push_back((s_T*F_y_B[ei] - s_B*F_y_T[ei] +
                                    s_T*s_B*(*(Q_T[ei]) - *(Q_B[ei])))/(s_T - s_B));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_T;
                    v_intercell = v_T + s_plus*((s_T - v_T)/(s_T - s_star) - 1);
                    w_intercell = w_T;
                }
                
                /*
                 * Calulate the weights beta for hybridization.
                 */
                
                double alpha1, alpha2;
                
                double vel_mag = sqrt(pow(u_T - u_B, 2) + pow(v_T - v_B, 2) + pow(w_T - w_B, 2));
                
                if (vel_mag < 1e-40)
                {
                    alpha1 = 1.0;
                    alpha2 = 0.0;
                }
                else
                {
                    alpha1 = fabs(v_T - v_B)/vel_mag;
                    alpha2 = sqrt(1.0 - alpha1*alpha1);
                }
                
                double beta1 = 0.5 + 0.5*alpha1/(alpha1 + alpha2);
                double beta2 = 1.0 - beta1;
                
                /*
                 * Compute the HLLC-HLL flux.
                 */
                
                *(F_y_intercell[0]) = beta1*F_y_intercell_HLLC[0] + beta2*F_y_intercell_HLL[0];
                *(F_y_intercell[1]) = beta1*F_y_intercell_HLLC[1] + beta2*F_y_intercell_HLL[1];
                *(F_y_intercell[2]) = F_y_intercell_HLLC[2];
                *(F_y_intercell[3]) = beta1*F_y_intercell_HLLC[3] + beta2*F_y_intercell_HLL[3];
                *(F_y_intercell[4]) = F_y_intercell_HLLC[4];
                for (int si = 0; si < d_num_species - 1; si++)
                {
                    *(F_y_intercell[5 + si]) = beta1*F_y_intercell_HLLC[5 + si] + beta2*F_y_intercell_HLL[5 + si];
                    // *(F_y_intercell[5 + si]) = F_y_intercell_HLLC[5 + si];
                }
                
                break;
            }
            case Z_DIRECTION:
            {
                std::vector<double*> F_z_intercell = flux_intercell;
                
                std::vector<double> F_z_intercell_HLLC;
                std::vector<double> F_z_intercell_HLL;
                
                double& u_intercell = *(velocity_intercell[0]);
                double& v_intercell = *(velocity_intercell[1]);
                double& w_intercell = *(velocity_intercell[2]);
                
                const double& rho_B = *density_L;
                const double& rho_F = *density_R;
                
                const double& rho_u_B = *(momentum_L[0]);
                const double& rho_u_F = *(momentum_R[0]);
                
                const double& rho_v_B = *(momentum_L[1]);
                const double& rho_v_F = *(momentum_R[1]);
                
                const double& rho_w_B = *(momentum_L[2]);
                const double& rho_w_F = *(momentum_R[2]);
                
                const std::vector<const double*> m_B = momentum_L;
                const std::vector<const double*> m_F = momentum_R;
                
                const double& u_B = rho_u_B/rho_B;
                const double& u_F = rho_u_F/rho_F;
                
                const double& v_B = rho_v_B/rho_B;
                const double& v_F = rho_v_F/rho_F;
                
                const double& w_B = rho_w_B/rho_B;
                const double& w_F = rho_w_F/rho_F;
                
                const double& E_B = *total_energy_L;
                const double& E_F = *total_energy_R;
                
                const std::vector<const double*> Y_B = mass_fraction_L;
                const std::vector<const double*> Y_F = mass_fraction_R;
                
                const double p_B = d_equation_of_state->
                    getPressureWithMassFraction(
                        &rho_B,
                        m_B,
                        &E_B,
                        Y_B);
                
                const double p_F = d_equation_of_state->
                    getPressureWithMassFraction(
                        &rho_F,
                        m_F,
                        &E_F,
                        Y_F);
                
                const double c_B = d_equation_of_state->
                    getSoundSpeedWithMassFractionAndPressure(
                        &rho_B,
                        Y_B,
                        &p_B);
                
                const double c_F = d_equation_of_state->
                    getSoundSpeedWithMassFractionAndPressure(
                        &rho_F,
                        Y_F,
                        &p_F);
                
                const double w_average = 0.5*(w_B + w_F);
                const double c_average = 0.5*(c_F + c_F);
                
                const double s_B = fmin(w_average - c_average, w_B - c_B);
                const double s_F = fmax(w_average + c_average, w_F + c_F);
                
                const double s_minus = fmin(0.0, s_B);
                const double s_plus  = fmax(0.0, s_F);
                
                const double s_star =
                    (p_F - p_B + rho_B*w_B*(s_B - w_B) - rho_F*w_F*(s_F - w_F))/(rho_B*(s_B - w_B) -
                        rho_F*(s_F - w_F));
                
                if (s_star > 0)
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_B = (s_B - w_B)/(s_B - s_star);    
                    
                    std::vector<double> Q_star_B;
                    Q_star_B.push_back(Chi_star_B*rho_B);
                    Q_star_B.push_back(Chi_star_B*rho_u_B);
                    Q_star_B.push_back(Chi_star_B*rho_v_B);
                    Q_star_B.push_back(Chi_star_B*rho_B*s_star);
                    Q_star_B.push_back(Chi_star_B*(E_B + (s_star - w_B)*(rho_B*s_star + p_B/(s_B - w_B))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_B.push_back(Chi_star_B*(*(Y_B[si])));
                    }
                    
                    std::vector<const double*> Q_B;
                    Q_B.push_back(&rho_B);
                    Q_B.push_back(m_B[0]);
                    Q_B.push_back(m_B[1]);
                    Q_B.push_back(m_B[2]);
                    Q_B.push_back(&E_B);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_B.push_back(Y_B[si]);
                    }
                    
                    std::vector<double> F_z_B;
                    F_z_B.push_back(rho_w_B);
                    F_z_B.push_back(rho_w_B*u_B);
                    F_z_B.push_back(rho_w_B*v_B);
                    F_z_B.push_back(rho_w_B*w_B + p_B);
                    F_z_B.push_back(w_B*(E_B + p_B));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_z_B.push_back(w_B*(*(Y_B[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_z_intercell_HLLC.push_back(F_z_B[ei] + s_minus*(Q_star_B[ei] - *(Q_B[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_B >= 0)
                    {
                        F_z_intercell_HLL = F_z_B;
                    }
                    else
                    {
                        std::vector<double> F_z_F;
                        F_z_F.push_back(rho_w_F);
                        F_z_F.push_back(rho_w_F*u_F);
                        F_z_F.push_back(rho_w_F*v_F);
                        F_z_F.push_back(rho_w_F*w_F + p_F);
                        F_z_F.push_back(w_F*(E_F + p_F));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_z_F.push_back(w_F*(*(Y_F[si])));
                        }
                        
                        if (s_F <= 0)
                        {
                            F_z_intercell_HLL = F_z_F;
                        }
                        else
                        {
                            std::vector<const double*> Q_F;
                            Q_F.push_back(&rho_F);
                            Q_F.push_back(m_F[0]);
                            Q_F.push_back(m_F[1]);
                            Q_F.push_back(m_F[2]);
                            Q_F.push_back(&E_F);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_F.push_back(Y_F[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_z_intercell_HLL.push_back((s_F*F_z_B[ei] - s_B*F_z_F[ei] +
                                    s_F*s_B*(*(Q_F[ei]) - *(Q_B[ei])))/(s_F - s_B));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    u_intercell = u_B;
                    v_intercell = v_B;
                    w_intercell = w_B + s_minus*((s_B - w_B)/(s_B - s_star) - 1);
                }
                else
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_F = (s_F - w_F)/(s_F - s_star); 
                    
                    std::vector<double> Q_star_F;
                    Q_star_F.push_back(Chi_star_F*rho_F);
                    Q_star_F.push_back(Chi_star_F*rho_u_F);
                    Q_star_F.push_back(Chi_star_F*rho_v_F);
                    Q_star_F.push_back(Chi_star_F*rho_F*s_star);
                    Q_star_F.push_back(Chi_star_F*(E_F + (s_star - w_F)*(rho_F*s_star + p_F/(s_F - w_F))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_F.push_back(Chi_star_F*(*(Y_F[si])));
                    }
                    
                    std::vector<const double*> Q_F;
                    Q_F.push_back(&rho_F);
                    Q_F.push_back(m_F[0]);
                    Q_F.push_back(m_F[1]);
                    Q_F.push_back(m_F[2]);
                    Q_F.push_back(&E_F);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_F.push_back(Y_F[si]);
                    }
                    
                    std::vector<double> F_z_F;
                    F_z_F.push_back(rho_w_F);
                    F_z_F.push_back(rho_w_F*u_F);
                    F_z_F.push_back(rho_w_F*v_F);
                    F_z_F.push_back(rho_w_F*w_F + p_F);
                    F_z_F.push_back(w_F*(E_F + p_F));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_z_F.push_back(w_F*(*(Y_F[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_z_intercell_HLLC.push_back(F_z_F[ei] + s_plus*(Q_star_F[ei] - *(Q_F[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_F <= 0)
                    {
                        F_z_intercell_HLL = F_z_F;
                    }
                    else
                    {
                        std::vector<double> F_z_B;
                        F_z_B.push_back(rho_w_B);
                        F_z_B.push_back(rho_w_B*u_B);
                        F_z_B.push_back(rho_w_B*v_B);
                        F_z_B.push_back(rho_w_B*w_B + p_B);
                        F_z_B.push_back(w_B*(E_B + p_B));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_z_B.push_back(w_B*(*(Y_B[si])));
                        }
                        
                        if (s_B >= 0)
                        {
                            F_z_intercell_HLL = F_z_B;
                        }
                        else
                        {
                            std::vector<const double*> Q_B;
                            Q_B.push_back(&rho_B);
                            Q_B.push_back(m_B[0]);
                            Q_B.push_back(m_B[1]);
                            Q_B.push_back(m_B[2]);
                            Q_B.push_back(&E_B);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_B.push_back(Y_B[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_z_intercell_HLL.push_back((s_F*F_z_B[ei] - s_B*F_z_F[ei] +
                                    s_F*s_B*(*(Q_F[ei]) - *(Q_B[ei])))/(s_F - s_B));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_F;
                    v_intercell = v_F;
                    w_intercell = w_F + s_plus*((s_F - w_F)/(s_F - s_star) - 1);
                }
                
                /*
                 * Calulate the weights beta for hybridization.
                 */
                
                double alpha1, alpha2;
                
                double vel_mag = sqrt(pow(u_F - u_B, 2) + pow(v_F - v_B, 2) + pow(w_F - w_B, 2));
                
                if (vel_mag < 1e-40)
                {
                    alpha1 = 1.0;
                    alpha2 = 0.0;
                }
                else
                {
                    alpha1 = fabs(w_F - w_B)/vel_mag;
                    alpha2 = sqrt(1.0 - alpha1*alpha1);
                }
                
                double beta1 = 0.5 + 0.5*alpha1/(alpha1 + alpha2);
                double beta2 = 1.0 - beta1;
                
                /*
                 * Compute the HLLC-HLL flux.
                 */
                
                *(F_z_intercell[0]) = beta1*F_z_intercell_HLLC[0] + beta2*F_z_intercell_HLL[0];
                *(F_z_intercell[1]) = beta1*F_z_intercell_HLLC[1] + beta2*F_z_intercell_HLL[1];
                *(F_z_intercell[2]) = beta1*F_z_intercell_HLLC[2] + beta2*F_z_intercell_HLL[2];
                *(F_z_intercell[3]) = F_z_intercell_HLLC[3];
                *(F_z_intercell[4]) = F_z_intercell_HLLC[4];
                for (int si = 0; si < d_num_species - 1; si++)
                {
                    *(F_z_intercell[5 + si]) = beta1*F_z_intercell_HLLC[5 + si] + beta2*F_z_intercell_HLL[5 + si];
                    // *(F_z_intercell[5 + si]) = F_z_intercell_HLLC[5 + si];
                }
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "Unknown direction."
                           << std::endl);
            }
        }
    }
}


/*
 * Compute the fluxes and and velocities at the intercell faces
 * for five-equation multi-species flow model by Allaire.
 */
void
RiemannSolverHLLC_HLL::computeIntercellFluxAndVelocityForFiveEqnAllaire(
    std::vector<double*> flux_intercell,
    std::vector<double*> velocity_intercell,
    const std::vector<const double*> partial_density_L,
    const std::vector<const double*> partial_density_R,
    const std::vector<const double*> momentum_L,
    const std::vector<const double*> momentum_R,
    const double* const total_energy_L,
    const double* const total_energy_R,
    const std::vector<const double*> volume_fraction_L,
    const std::vector<const double*> volume_fraction_R,
    DIRECTION direction)
{
    if (d_dim == tbox::Dimension(1))
    {
#ifdef DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(static_cast<int>(flux_intercell.size()) == d_num_eqn);
        TBOX_ASSERT(static_cast<int>(velocity_intercell.size()) == 1);
        TBOX_ASSERT(static_cast<int>(partial_density_L.size()) == d_num_species);
        TBOX_ASSERT(static_cast<int>(partial_density_R.size()) == d_num_species);
        TBOX_ASSERT(static_cast<int>(momentum_L.size()) == 1);
        TBOX_ASSERT(static_cast<int>(momentum_R.size()) == 1);
        TBOX_ASSERT(static_cast<int>(volume_fraction_L.size()) == d_num_species - 1);
        TBOX_ASSERT(static_cast<int>(volume_fraction_R.size()) == d_num_species - 1);
        
        for (int fi = 0; fi < static_cast<int>(flux_intercell.size()); fi++)
        {
            TBOX_ASSERT(flux_intercell[fi]);
        }
        for (int vi = 0; vi < static_cast<int>(velocity_intercell.size()); vi++)
        {
            TBOX_ASSERT(velocity_intercell[vi]);
        }
        for (int pi = 0; pi < static_cast<int>(partial_density_L.size()); pi++)
        {
            TBOX_ASSERT(partial_density_L[pi]);
        }
        for (int pi = 0; pi < static_cast<int>(partial_density_R.size()); pi++)
        {
            TBOX_ASSERT(partial_density_R[pi]);
        }
        for (int mi = 0; mi < static_cast<int>(momentum_L.size()); mi++)
        {
            TBOX_ASSERT(momentum_L[mi]);
        }
        for (int mi = 0; mi < static_cast<int>(momentum_R.size()); mi++)
        {
            TBOX_ASSERT(momentum_R[mi]);
        }
        TBOX_ASSERT(total_energy_L);
        TBOX_ASSERT(total_energy_R);
        for (int vi = 0; vi < static_cast<int>(volume_fraction_L.size()); vi++)
        {
            TBOX_ASSERT(volume_fraction_L[vi]);
        }
        for (int vi = 0; vi < static_cast<int>(volume_fraction_R.size()); vi++)
        {
            TBOX_ASSERT(volume_fraction_R[vi]);
        }
#endif
        
        switch (direction)
        {
            case X_DIRECTION:
            {
                std::vector<double*> F_x_intercell = flux_intercell;
                
                double& u_intercell = *(velocity_intercell[0]);
                
                const std::vector<const double*> Z_rho_L = partial_density_L;
                const std::vector<const double*> Z_rho_R = partial_density_R;
                
                const double rho_L = d_equation_of_state->getTotalDensity(
                    partial_density_L);
                
                const double rho_R = d_equation_of_state->getTotalDensity(
                    partial_density_R);
                
                const double& rho_u_L = *(momentum_L[0]);
                const double& rho_u_R = *(momentum_R[0]);
                
                const std::vector<const double*> m_L = momentum_L;
                const std::vector<const double*> m_R = momentum_R;
                
                const double& u_L = rho_u_L/rho_L;
                const double& u_R = rho_u_R/rho_R;
                
                const double& E_L = *total_energy_L;
                const double& E_R = *total_energy_R;
                
                const std::vector<const double*> Z_L = volume_fraction_L;
                const std::vector<const double*> Z_R = volume_fraction_R;
                
                const double p_L = d_equation_of_state->getPressureWithVolumeFraction(
                    &rho_L,
                    m_L,
                    &E_L,
                    Z_L);
                
                const double p_R = d_equation_of_state->getPressureWithVolumeFraction(
                    &rho_R,
                    m_R,
                    &E_R,
                    Z_R);
                
                const double c_L = d_equation_of_state->getSoundSpeedWithVolumeFractionAndPressure(
                    &rho_L,
                    Z_L,
                    &p_L);
                
                const double c_R = d_equation_of_state->getSoundSpeedWithVolumeFractionAndPressure(
                    &rho_R,
                    Z_R,
                    &p_R);
                
                const double u_average = 0.5*(u_L + u_R);
                const double c_average = 0.5*(c_L + c_R);
                
                const double s_L = fmin(u_average - c_average, u_L - c_L);
                const double s_R = fmax(u_average + c_average, u_R + c_R);
                
                const double s_minus = fmin(0.0, s_L);
                const double s_plus  = fmax(0.0, s_R);
                
                const double s_star =
                    (p_R - p_L + rho_L*u_L*(s_L - u_L) - rho_R*u_R*(s_R - u_R))/(rho_L*(s_L - u_L) -
                        rho_R*(s_R - u_R));
                
                if (s_star > 0)
                {
                    const double Chi_star_L = (s_L - u_L)/(s_L - s_star);
                    
                    std::vector<double> Q_star_L;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_star_L.push_back(Chi_star_L*(*(Z_rho_L[si])));
                    }
                    Q_star_L.push_back(Chi_star_L*rho_L*s_star);
                    Q_star_L.push_back(Chi_star_L*(E_L + (s_star - u_L)*(rho_L*s_star + p_L/(s_L - u_L))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_L.push_back(Chi_star_L*(*(Z_L[si])));
                    }
                    
                    std::vector<const double*> Q_L;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_L.push_back(Z_rho_L[si]);
                    }
                    Q_L.push_back(m_L[0]);
                    Q_L.push_back(&E_L);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_L.push_back(Z_L[si]);
                    }
                    
                    std::vector<double> F_x_L;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        F_x_L.push_back(u_L*(*(Z_rho_L[si])));
                    }
                    F_x_L.push_back(rho_u_L*u_L + p_L);
                    F_x_L.push_back(u_L*(E_L + p_L));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_x_L.push_back(u_L*(*(Z_L[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        *(F_x_intercell[ei]) = F_x_L[ei] + s_minus*(Q_star_L[ei] - *(Q_L[ei]));
                    }
                    
                    u_intercell = u_L + s_minus*((s_L - u_L)/(s_L - s_star) - 1);
                }
                else
                {
                    const double Chi_star_R = (s_R - u_R)/(s_R - s_star);
                    
                    std::vector<double> Q_star_R;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_star_R.push_back(Chi_star_R*(*(Z_rho_R[si])));
                    }
                    Q_star_R.push_back(Chi_star_R*rho_R*s_star);
                    Q_star_R.push_back(Chi_star_R*(E_R + (s_star - u_R)*(rho_R*s_star + p_R/(s_R - u_R))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_R.push_back(Chi_star_R*(*(Z_R[si])));
                    }
                    
                    std::vector<const double*> Q_R;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_R.push_back(Z_rho_R[si]);
                    }
                    Q_R.push_back(m_R[0]);
                    Q_R.push_back(&E_R);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_R.push_back(Z_R[si]);
                    }
                    
                    std::vector<double> F_x_R;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        F_x_R.push_back(u_R*(*(Z_rho_R[si])));
                    }
                    F_x_R.push_back(rho_u_R*u_R + p_R);
                    F_x_R.push_back(u_R*(E_R + p_R));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_x_R.push_back(u_R*(*(Z_R[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        *(F_x_intercell[ei]) = F_x_R[ei] + s_plus*(Q_star_R[ei] - *(Q_R[ei]));
                    }
                    
                    u_intercell = u_R + s_plus*((s_R - u_R)/(s_R - s_star) - 1);
                }
                
                break;
            }
            case Y_DIRECTION:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "There is no y direction for 1D problem."
                           << std::endl);
                break;
            }
            case Z_DIRECTION:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "There is no z direction for 1D problem."
                           << std::endl);
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "Unknown direction."
                           << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(2))
    {
#ifdef DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(static_cast<int>(flux_intercell.size()) == d_num_eqn);
        TBOX_ASSERT(static_cast<int>(velocity_intercell.size()) == 2);
        TBOX_ASSERT(static_cast<int>(partial_density_L.size()) == d_num_species);
        TBOX_ASSERT(static_cast<int>(partial_density_R.size()) == d_num_species);
        TBOX_ASSERT(static_cast<int>(momentum_L.size()) == 2);
        TBOX_ASSERT(static_cast<int>(momentum_R.size()) == 2);
        TBOX_ASSERT(static_cast<int>(volume_fraction_L.size()) == d_num_species - 1);
        TBOX_ASSERT(static_cast<int>(volume_fraction_R.size()) == d_num_species - 1);
        
        for (int fi = 0; fi < static_cast<int>(flux_intercell.size()); fi++)
        {
            TBOX_ASSERT(flux_intercell[fi]);
        }
        for (int vi = 0; vi < static_cast<int>(velocity_intercell.size()); vi++)
        {
            TBOX_ASSERT(velocity_intercell[vi]);
        }
        for (int pi = 0; pi < static_cast<int>(partial_density_L.size()); pi++)
        {
            TBOX_ASSERT(partial_density_L[pi]);
        }
        for (int pi = 0; pi < static_cast<int>(partial_density_R.size()); pi++)
        {
            TBOX_ASSERT(partial_density_R[pi]);
        }
        for (int mi = 0; mi < static_cast<int>(momentum_L.size()); mi++)
        {
            TBOX_ASSERT(momentum_L[mi]);
        }
        for (int mi = 0; mi < static_cast<int>(momentum_R.size()); mi++)
        {
            TBOX_ASSERT(momentum_R[mi]);
        }
        TBOX_ASSERT(total_energy_L);
        TBOX_ASSERT(total_energy_R);
        for (int vi = 0; vi < static_cast<int>(volume_fraction_L.size()); vi++)
        {
            TBOX_ASSERT(volume_fraction_L[vi]);
        }
        for (int vi = 0; vi < static_cast<int>(volume_fraction_R.size()); vi++)
        {
            TBOX_ASSERT(volume_fraction_R[vi]);
        }
#endif
        
        switch (direction)
        {
            case X_DIRECTION:
            {
                std::vector<double*> F_x_intercell = flux_intercell;
                
                std::vector<double> F_x_intercell_HLLC;
                std::vector<double> F_x_intercell_HLL;
                
                double& u_intercell = *(velocity_intercell[0]);
                double& v_intercell = *(velocity_intercell[1]);
                
                const std::vector<const double*> Z_rho_L = partial_density_L;
                const std::vector<const double*> Z_rho_R = partial_density_R;
                
                const double rho_L = d_equation_of_state->getTotalDensity(
                    partial_density_L);
                
                const double rho_R = d_equation_of_state->getTotalDensity(
                    partial_density_R);
                
                const double& rho_u_L = *(momentum_L[0]);
                const double& rho_u_R = *(momentum_R[0]);
                
                const double& rho_v_L = *(momentum_L[1]);
                const double& rho_v_R = *(momentum_R[1]);
                
                const std::vector<const double*> m_L = momentum_L;
                const std::vector<const double*> m_R = momentum_R;
                
                const double& u_L = rho_u_L/rho_L;
                const double& u_R = rho_u_R/rho_R;
                
                const double& v_L = rho_v_L/rho_L;
                const double& v_R = rho_v_R/rho_R;
                
                const double& E_L = *total_energy_L;
                const double& E_R = *total_energy_R;
                
                const std::vector<const double*> Z_L = volume_fraction_L;
                const std::vector<const double*> Z_R = volume_fraction_R;
                
                const double p_L = d_equation_of_state->getPressureWithVolumeFraction(
                    &rho_L,
                    m_L,
                    &E_L,
                    Z_L);
                
                const double p_R = d_equation_of_state->getPressureWithVolumeFraction(
                    &rho_R,
                    m_R,
                    &E_R,
                    Z_R);
                
                const double c_L = d_equation_of_state->getSoundSpeedWithVolumeFractionAndPressure(
                    &rho_L,
                    Z_L,
                    &p_L);
                
                const double c_R = d_equation_of_state->getSoundSpeedWithVolumeFractionAndPressure(
                    &rho_R,
                    Z_R,
                    &p_R);
                
                const double u_average = 0.5*(u_L + u_R);
                const double c_average = 0.5*(c_L + c_R);
                
                const double s_L = fmin(u_average - c_average, u_L - c_L);
                const double s_R = fmax(u_average + c_average, u_R + c_R);
                
                const double s_minus = fmin(0.0, s_L);
                const double s_plus  = fmax(0.0, s_R);
                
                const double s_star =
                    (p_R - p_L + rho_L*u_L*(s_L - u_L) - rho_R*u_R*(s_R - u_R))/(rho_L*(s_L - u_L) -
                        rho_R*(s_R - u_R));
                
                if (s_star > 0)
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_L = (s_L - u_L)/(s_L - s_star);
                    
                    std::vector<double> Q_star_L;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_star_L.push_back(Chi_star_L*(*(Z_rho_L[si])));
                    }
                    Q_star_L.push_back(Chi_star_L*rho_L*s_star);
                    Q_star_L.push_back(Chi_star_L*rho_v_L);
                    Q_star_L.push_back(Chi_star_L*(E_L + (s_star - u_L)*(rho_L*s_star + p_L/(s_L - u_L))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_L.push_back(Chi_star_L*(*(Z_L[si])));
                    }
                    
                    std::vector<const double*> Q_L;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_L.push_back(Z_rho_L[si]);
                    }
                    Q_L.push_back(m_L[0]);
                    Q_L.push_back(m_L[1]);
                    Q_L.push_back(&E_L);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_L.push_back(Z_L[si]);
                    }
                    
                    std::vector<double> F_x_L;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        F_x_L.push_back(u_L*(*(Z_rho_L[si])));
                    }
                    F_x_L.push_back(rho_u_L*u_L + p_L);
                    F_x_L.push_back(rho_u_L*v_L);
                    F_x_L.push_back(u_L*(E_L + p_L));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_x_L.push_back(u_L*(*(Z_L[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_x_intercell_HLLC.push_back(F_x_L[ei] + s_minus*(Q_star_L[ei] - *(Q_L[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_L >= 0)
                    {
                        F_x_intercell_HLL = F_x_L;
                    }
                    else
                    {
                        std::vector<double> F_x_R;
                        for (int si = 0; si < d_num_species; si++)
                        {
                            F_x_R.push_back(u_R*(*(Z_rho_R[si])));
                        }
                        F_x_R.push_back(rho_u_R*u_R + p_R);
                        F_x_R.push_back(rho_u_R*v_R);
                        F_x_R.push_back(u_R*(E_R + p_R));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_x_R.push_back(u_R*(*(Z_R[si])));
                        }
                        
                        if (s_R <= 0)
                        {
                            F_x_intercell_HLL = F_x_R;
                        }
                        else
                        {
                            std::vector<const double*> Q_R;
                            for (int si = 0; si < d_num_species; si++)
                            {
                                Q_R.push_back(Z_rho_R[si]);
                            }
                            Q_R.push_back(m_R[0]);
                            Q_R.push_back(m_R[1]);
                            Q_R.push_back(&E_R);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_R.push_back(Z_R[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_x_intercell_HLL.push_back((s_R*F_x_L[ei] - s_L*F_x_R[ei] +
                                    s_R*s_L*(*(Q_R[ei]) - *(Q_L[ei])))/(s_R - s_L));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_L + s_minus*((s_L - u_L)/(s_L - s_star) - 1);
                    v_intercell = v_L;
                }
                else
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_R = (s_R - u_R)/(s_R - s_star);
                    
                    std::vector<double> Q_star_R;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_star_R.push_back(Chi_star_R*(*(Z_rho_R[si])));
                    }
                    Q_star_R.push_back(Chi_star_R*rho_R*s_star);
                    Q_star_R.push_back(Chi_star_R*rho_v_R);
                    Q_star_R.push_back(Chi_star_R*(E_R + (s_star - u_R)*(rho_R*s_star + p_R/(s_R - u_R))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_R.push_back(Chi_star_R*(*(Z_R[si])));
                    }
                    
                    std::vector<const double*> Q_R;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_R.push_back(Z_rho_R[si]);
                    }
                    Q_R.push_back(m_R[0]);
                    Q_R.push_back(m_R[1]);
                    Q_R.push_back(&E_R);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_R.push_back(Z_R[si]);
                    }
                    
                    std::vector<double> F_x_R;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        F_x_R.push_back(u_R*(*(Z_rho_R[si])));
                    }
                    F_x_R.push_back(rho_u_R*u_R + p_R);
                    F_x_R.push_back(rho_u_R*v_R);
                    F_x_R.push_back(u_R*(E_R + p_R));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_x_R.push_back(u_R*(*(Z_R[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_x_intercell_HLLC.push_back(F_x_R[ei] + s_plus*(Q_star_R[ei] - *(Q_R[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_R <= 0)
                    {
                        F_x_intercell_HLL = F_x_R;
                    }
                    else
                    {
                        std::vector<double> F_x_L;
                        for (int si = 0; si < d_num_species; si++)
                        {
                            F_x_L.push_back(u_L*(*(Z_rho_L[si])));
                        }
                        F_x_L.push_back(rho_u_L*u_L + p_L);
                        F_x_L.push_back(rho_u_L*v_L);
                        F_x_L.push_back(u_L*(E_L + p_L));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_x_L.push_back(u_L*(*(Z_L[si])));
                        }
                        
                        if (s_L >= 0)
                        {
                            F_x_intercell_HLL = F_x_L;
                        }
                        else
                        {
                            std::vector<const double*> Q_L;
                            for (int si = 0; si < d_num_species; si++)
                            {
                                Q_L.push_back(Z_rho_L[si]);
                            }
                            Q_L.push_back(m_L[0]);
                            Q_L.push_back(m_L[1]);
                            Q_L.push_back(&E_L);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_L.push_back(Z_L[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_x_intercell_HLL.push_back((s_R*F_x_L[ei] - s_L*F_x_R[ei] +
                                    s_R*s_L*(*(Q_R[ei]) - *(Q_L[ei])))/(s_R - s_L));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_R + s_plus*((s_R - u_R)/(s_R - s_star) - 1);
                    v_intercell = v_R;
                }
                
                /*
                 * Calulate the weights beta for hybridization.
                 */
                
                double alpha1, alpha2;
                
                double vel_mag = sqrt(pow(u_R - u_L, 2) + pow(v_R - v_L, 2));
                
                if (vel_mag < 1e-40)
                {
                    alpha1 = 1.0;
                    alpha2 = 0.0;
                }
                else
                {
                    alpha1 = fabs(u_R - u_L)/vel_mag;
                    alpha2 = sqrt(1.0 - alpha1*alpha1);
                }
                
                double beta1 = 0.5 + 0.5*alpha1/(alpha1 + alpha2);
                double beta2 = 1.0 - beta1;
                
                /*
                 * Compute the HLLC-HLL flux.
                 */
                
                for (int si = 0; si < d_num_species; si++)
                {
                    *(F_x_intercell[si]) = beta1*F_x_intercell_HLLC[si] + beta2*F_x_intercell_HLL[si];
                }
                *(F_x_intercell[d_num_species]) = F_x_intercell_HLLC[d_num_species];
                *(F_x_intercell[d_num_species + 1]) = beta1*F_x_intercell_HLLC[d_num_species + 1] +
                    beta2*F_x_intercell_HLL[d_num_species + 1];
                *(F_x_intercell[d_num_species + 2]) = F_x_intercell_HLLC[d_num_species + 2];
                for (int si = 0; si < d_num_species - 1; si++)
                {
                    *(F_x_intercell[d_num_species + 3 + si]) = beta1*F_x_intercell_HLLC[d_num_species + 3 + si] +
                        beta2*F_x_intercell_HLL[d_num_species + 3 + si];
                    // *(F_x_intercell[d_num_species + 3 + si]) = F_x_intercell_HLLC[d_num_species + 3 + si];
                }
                
                break;
            }
            case Y_DIRECTION:
            {
                std::vector<double*> F_y_intercell = flux_intercell;
                
                std::vector<double> F_y_intercell_HLLC;
                std::vector<double> F_y_intercell_HLL;
                
                double& u_intercell = *(velocity_intercell[0]);
                double& v_intercell = *(velocity_intercell[1]);
                
                const std::vector<const double*> Z_rho_B = partial_density_L;
                const std::vector<const double*> Z_rho_T = partial_density_R;
                
                const double rho_B = d_equation_of_state->getTotalDensity(
                    partial_density_L);
                
                const double rho_T = d_equation_of_state->getTotalDensity(
                    partial_density_R);
                
                const double& rho_u_B = *(momentum_L[0]);
                const double& rho_u_T = *(momentum_R[0]);
                
                const double& rho_v_B = *(momentum_L[1]);
                const double& rho_v_T = *(momentum_R[1]);
                
                const std::vector<const double*> m_B = momentum_L;
                const std::vector<const double*> m_T = momentum_R;
                
                const double& u_B = rho_u_B/rho_B;
                const double& u_T = rho_u_T/rho_T;
                
                const double& v_B = rho_v_B/rho_B;
                const double& v_T = rho_v_T/rho_T;
                
                const double& E_B = *total_energy_L;
                const double& E_T = *total_energy_R;
                
                const std::vector<const double*> Z_B = volume_fraction_L;
                const std::vector<const double*> Z_T = volume_fraction_R;
                
                const double p_B = d_equation_of_state->getPressureWithVolumeFraction(
                    &rho_B,
                    m_B,
                    &E_B,
                    Z_B);
                
                const double p_T = d_equation_of_state->getPressureWithVolumeFraction(
                    &rho_T,
                    m_T,
                    &E_T,
                    Z_T);
                
                const double c_B = d_equation_of_state->getSoundSpeedWithVolumeFractionAndPressure(
                    &rho_B,
                    Z_B,
                    &p_B);
                
                const double c_T = d_equation_of_state->getSoundSpeedWithVolumeFractionAndPressure(
                    &rho_T,
                    Z_T,
                    &p_T);
                
                const double v_average = 0.5*(v_B + v_T);
                const double c_average = 0.5*(c_B + c_T);
                
                const double s_B = fmin(v_average - c_average, v_B - c_B);
                const double s_T = fmax(v_average + c_average, v_T + c_T);
                
                const double s_minus = fmin(0.0, s_B);
                const double s_plus  = fmax(0.0, s_T);
                
                const double s_star =
                    (p_T - p_B + rho_B*v_B*(s_B - v_B) - rho_T*v_T*(s_T - v_T))/(rho_B*(s_B - v_B) -
                        rho_T*(s_T - v_T));
                
                if (s_star > 0)
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_B = (s_B - v_B)/(s_B - s_star);    
                    
                    std::vector<double> Q_star_B;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_star_B.push_back(Chi_star_B*(*(Z_rho_B[si])));
                    }
                    Q_star_B.push_back(Chi_star_B*rho_u_B);
                    Q_star_B.push_back(Chi_star_B*rho_B*s_star);
                    Q_star_B.push_back(Chi_star_B*(E_B + (s_star - v_B)*(rho_B*s_star + p_B/(s_B - v_B))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_B.push_back(Chi_star_B*(*(Z_B[si])));
                    }
                    
                    std::vector<const double*> Q_B;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_B.push_back(Z_rho_B[si]);
                    }
                    Q_B.push_back(m_B[0]);
                    Q_B.push_back(m_B[1]);
                    Q_B.push_back(&E_B);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_B.push_back(Z_B[si]);
                    }
                    
                    std::vector<double> F_y_B;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        F_y_B.push_back(v_B*(*(Z_rho_B[si])));
                    }
                    F_y_B.push_back(rho_v_B*u_B);
                    F_y_B.push_back(rho_v_B*v_B + p_B);
                    F_y_B.push_back(v_B*(E_B + p_B));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_y_B.push_back(v_B*(*(Z_B[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_y_intercell_HLLC.push_back(F_y_B[ei] + s_minus*(Q_star_B[ei] - *(Q_B[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_B >= 0)
                    {
                        F_y_intercell_HLL = F_y_B;
                    }
                    else
                    {
                        std::vector<double> F_y_T;
                        for (int si = 0; si < d_num_species; si++)
                        {
                            F_y_T.push_back(v_T*(*(Z_rho_T[si])));
                        }
                        F_y_T.push_back(rho_v_T*u_T);
                        F_y_T.push_back(rho_v_T*v_T + p_T);
                        F_y_T.push_back(v_T*(E_T + p_T));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_y_T.push_back(v_T*(*(Z_T[si])));
                        }
                        
                        if (s_T <= 0)
                        {
                            F_y_intercell_HLL = F_y_T;
                        }
                        else
                        {
                            std::vector<const double*> Q_T;
                            for (int si = 0; si < d_num_species; si++)
                            {
                                Q_T.push_back(Z_rho_T[si]);
                            }
                            Q_T.push_back(m_T[0]);
                            Q_T.push_back(m_T[1]);
                            Q_T.push_back(&E_T);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_T.push_back(Z_T[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_y_intercell_HLL.push_back((s_T*F_y_B[ei] - s_B*F_y_T[ei] +
                                    s_T*s_B*(*(Q_T[ei]) - *(Q_B[ei])))/(s_T - s_B));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_B;
                    v_intercell = v_B + s_minus*((s_B - v_B)/(s_B - s_star) - 1);
                }
                else
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_T = (s_T - v_T)/(s_T - s_star); 
                    
                    std::vector<double> Q_star_T;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_star_T.push_back(Chi_star_T*(*(Z_rho_T[si])));
                    }
                    Q_star_T.push_back(Chi_star_T*rho_u_T);
                    Q_star_T.push_back(Chi_star_T*rho_T*s_star);
                    Q_star_T.push_back(Chi_star_T*(E_T + (s_star - v_T)*(rho_T*s_star + p_T/(s_T - v_T))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_T.push_back(Chi_star_T*(*(Z_T[si])));
                    }
                    
                    std::vector<const double*> Q_T;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_T.push_back(Z_rho_T[si]);
                    }
                    Q_T.push_back(m_T[0]);
                    Q_T.push_back(m_T[1]);
                    Q_T.push_back(&E_T);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_T.push_back(Z_T[si]);
                    }
                    
                    std::vector<double> F_y_T;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        F_y_T.push_back(v_T*(*(Z_rho_T[si])));
                    }
                    F_y_T.push_back(rho_v_T*u_T);
                    F_y_T.push_back(rho_v_T*v_T + p_T);
                    F_y_T.push_back(v_T*(E_T + p_T));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_y_T.push_back(v_T*(*(Z_T[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_y_intercell_HLLC.push_back(F_y_T[ei] + s_plus*(Q_star_T[ei] - *(Q_T[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_T <= 0)
                    {
                        F_y_intercell_HLL = F_y_T;
                    }
                    else
                    {
                        std::vector<double> F_y_B;
                        for (int si = 0; si < d_num_species; si++)
                        {
                            F_y_B.push_back(v_B*(*(Z_rho_B[si])));
                        }
                        F_y_B.push_back(rho_v_B*u_B);
                        F_y_B.push_back(rho_v_B*v_B + p_B);
                        F_y_B.push_back(v_B*(E_B + p_B));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_y_B.push_back(v_B*(*(Z_B[si])));
                        }
                        
                        if (s_B >= 0)
                        {
                            F_y_intercell_HLL = F_y_B;
                        }
                        else
                        {
                            std::vector<const double*> Q_B;
                            for (int si = 0; si < d_num_species; si++)
                            {
                                Q_B.push_back(Z_rho_B[si]);
                            }
                            Q_B.push_back(m_B[0]);
                            Q_B.push_back(m_B[1]);
                            Q_B.push_back(&E_B);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_B.push_back(Z_B[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_y_intercell_HLL.push_back((s_T*F_y_B[ei] - s_B*F_y_T[ei] +
                                    s_T*s_B*(*(Q_T[ei]) - *(Q_B[ei])))/(s_T - s_B));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_T;
                    v_intercell = v_T + s_plus*((s_T - v_T)/(s_T - s_star) - 1);
                }
                
                /*
                 * Calulate the weights beta for hybridization.
                 */
                
                double alpha1, alpha2;
                
                double vel_mag = sqrt(pow(u_T - u_B, 2) + pow(v_T - v_B, 2));
                
                if (vel_mag < 1e-40)
                {
                    alpha1 = 1.0;
                    alpha2 = 0.0;
                }
                else
                {
                    alpha1 = fabs(v_T - v_B)/vel_mag;
                    alpha2 = sqrt(1.0 - alpha1*alpha1);
                }
                
                double beta1 = 0.5 + 0.5*alpha1/(alpha1 + alpha2);
                double beta2 = 1.0 - beta1;
                
                /*
                 * Compute the HLLC-HLL flux.
                 */
                
                for (int si = 0; si < d_num_species; si++)
                {
                    *(F_y_intercell[si]) = beta1*F_y_intercell_HLLC[si] + beta2*F_y_intercell_HLL[si];
                }
                *(F_y_intercell[d_num_species]) = beta1*F_y_intercell_HLLC[d_num_species] +
                    beta2*F_y_intercell_HLL[d_num_species];
                *(F_y_intercell[d_num_species + 1]) = F_y_intercell_HLLC[d_num_species + 1];
                *(F_y_intercell[d_num_species + 2]) = F_y_intercell_HLLC[d_num_species + 2];
                for (int si = 0; si < d_num_species - 1; si++)
                {
                    *(F_y_intercell[d_num_species + 3 + si]) = beta1*F_y_intercell_HLLC[d_num_species + 3 + si] +
                        beta2*F_y_intercell_HLL[d_num_species + 3 + si];
                    // *(F_y_intercell[d_num_species + 3 + si]) = F_y_intercell_HLLC[d_num_species + 3 + si];
                    
                }
                
                break;
            }
            case Z_DIRECTION:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "There is no z direction for 1D problem."
                           << std::endl);
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "Unknown direction."
                           << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(3))
    {
#ifdef DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(static_cast<int>(flux_intercell.size()) == d_num_eqn);
        TBOX_ASSERT(static_cast<int>(velocity_intercell.size()) == 3);
        TBOX_ASSERT(static_cast<int>(partial_density_L.size()) == d_num_species);
        TBOX_ASSERT(static_cast<int>(partial_density_R.size()) == d_num_species);
        TBOX_ASSERT(static_cast<int>(momentum_L.size()) == 3);
        TBOX_ASSERT(static_cast<int>(momentum_R.size()) == 3);
        TBOX_ASSERT(static_cast<int>(volume_fraction_L.size()) == d_num_species - 1);
        TBOX_ASSERT(static_cast<int>(volume_fraction_R.size()) == d_num_species - 1);
        
        for (int fi = 0; fi < static_cast<int>(flux_intercell.size()); fi++)
        {
            TBOX_ASSERT(flux_intercell[fi]);
        }
        for (int vi = 0; vi < static_cast<int>(velocity_intercell.size()); vi++)
        {
            TBOX_ASSERT(velocity_intercell[vi]);
        }
        for (int pi = 0; pi < static_cast<int>(partial_density_L.size()); pi++)
        {
            TBOX_ASSERT(partial_density_L[pi]);
        }
        for (int pi = 0; pi < static_cast<int>(partial_density_R.size()); pi++)
        {
            TBOX_ASSERT(partial_density_R[pi]);
        }
        for (int mi = 0; mi < static_cast<int>(momentum_L.size()); mi++)
        {
            TBOX_ASSERT(momentum_L[mi]);
        }
        for (int mi = 0; mi < static_cast<int>(momentum_R.size()); mi++)
        {
            TBOX_ASSERT(momentum_R[mi]);
        }
        TBOX_ASSERT(total_energy_L);
        TBOX_ASSERT(total_energy_R);
        for (int vi = 0; vi < static_cast<int>(volume_fraction_L.size()); vi++)
        {
            TBOX_ASSERT(volume_fraction_L[vi]);
        }
        for (int vi = 0; vi < static_cast<int>(volume_fraction_R.size()); vi++)
        {
            TBOX_ASSERT(volume_fraction_R[vi]);
        }
#endif
        
        switch (direction)
        {
            case X_DIRECTION:
            {
                std::vector<double*> F_x_intercell = flux_intercell;
                
                std::vector<double> F_x_intercell_HLLC;
                std::vector<double> F_x_intercell_HLL;
                
                double& u_intercell = *(velocity_intercell[0]);
                double& v_intercell = *(velocity_intercell[1]);
                double& w_intercell = *(velocity_intercell[2]);
                
                const std::vector<const double*> Z_rho_L = partial_density_L;
                const std::vector<const double*> Z_rho_R = partial_density_R;
                
                const double rho_L = d_equation_of_state->getTotalDensity(
                    partial_density_L);
                
                const double rho_R = d_equation_of_state->getTotalDensity(
                    partial_density_R);
                
                const double& rho_u_L = *(momentum_L[0]);
                const double& rho_u_R = *(momentum_R[0]);
                
                const double& rho_v_L = *(momentum_L[1]);
                const double& rho_v_R = *(momentum_R[1]);
                
                const double& rho_w_L = *(momentum_L[2]);
                const double& rho_w_R = *(momentum_R[2]);
                
                const std::vector<const double*> m_L = momentum_L;
                const std::vector<const double*> m_R = momentum_R;
                
                const double& u_L = rho_u_L/rho_L;
                const double& u_R = rho_u_R/rho_R;
                
                const double& v_L = rho_v_L/rho_L;
                const double& v_R = rho_v_R/rho_R;
                
                const double& w_L = rho_w_L/rho_L;
                const double& w_R = rho_w_R/rho_R;
                
                const double& E_L = *total_energy_L;
                const double& E_R = *total_energy_R;
                
                const std::vector<const double*> Z_L = volume_fraction_L;
                const std::vector<const double*> Z_R = volume_fraction_R;
                
                const double p_L = d_equation_of_state->getPressureWithVolumeFraction(
                    &rho_L,
                    m_L,
                    &E_L,
                    Z_L);
                
                const double p_R = d_equation_of_state->getPressureWithVolumeFraction(
                    &rho_R,
                    m_R,
                    &E_R,
                    Z_R);
                
                const double c_L = d_equation_of_state->getSoundSpeedWithVolumeFractionAndPressure(
                    &rho_L,
                    Z_L,
                    &p_L);
                
                const double c_R = d_equation_of_state->getSoundSpeedWithVolumeFractionAndPressure(
                    &rho_R,
                    Z_R,
                    &p_R);
                
                const double u_average = 0.5*(u_L + u_R);
                const double c_average = 0.5*(c_L + c_R);
                
                const double s_L = fmin(u_average - c_average, u_L - c_L);
                const double s_R = fmax(u_average + c_average, u_R + c_R);
                
                const double s_minus = fmin(0.0, s_L);
                const double s_plus  = fmax(0.0, s_R);
                
                const double s_star =
                    (p_R - p_L + rho_L*u_L*(s_L - u_L) - rho_R*u_R*(s_R - u_R))/(rho_L*(s_L - u_L) -
                        rho_R*(s_R - u_R));
                
                if (s_star > 0)
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_L = (s_L - u_L)/(s_L - s_star);
                    
                    std::vector<double> Q_star_L;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_star_L.push_back(Chi_star_L*(*(Z_rho_L[si])));
                    }
                    Q_star_L.push_back(Chi_star_L*rho_L*s_star);
                    Q_star_L.push_back(Chi_star_L*rho_v_L);
                    Q_star_L.push_back(Chi_star_L*rho_w_L);
                    Q_star_L.push_back(Chi_star_L*(E_L + (s_star - u_L)*(rho_L*s_star + p_L/(s_L - u_L))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_L.push_back(Chi_star_L*(*(Z_L[si])));
                    }
                    
                    std::vector<const double*> Q_L;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_L.push_back(Z_rho_L[si]);
                    }
                    Q_L.push_back(m_L[0]);
                    Q_L.push_back(m_L[1]);
                    Q_L.push_back(m_L[2]);
                    Q_L.push_back(&E_L);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_L.push_back(Z_L[si]);
                    }
                    
                    std::vector<double> F_x_L;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        F_x_L.push_back(u_L*(*(Z_rho_L[si])));
                    }
                    F_x_L.push_back(rho_u_L*u_L + p_L);
                    F_x_L.push_back(rho_u_L*v_L);
                    F_x_L.push_back(rho_u_L*w_L);
                    F_x_L.push_back(u_L*(E_L + p_L));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_x_L.push_back(u_L*(*(Z_L[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_x_intercell_HLLC.push_back(F_x_L[ei] + s_minus*(Q_star_L[ei] - *(Q_L[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_L >= 0)
                    {
                        F_x_intercell_HLL = F_x_L;
                    }
                    else
                    {
                        std::vector<double> F_x_R;
                        for (int si = 0; si < d_num_species; si++)
                        {
                            F_x_R.push_back(u_R*(*(Z_rho_R[si])));
                        }
                        F_x_R.push_back(rho_u_R*u_R + p_R);
                        F_x_R.push_back(rho_u_R*v_R);
                        F_x_R.push_back(rho_u_R*w_R);
                        F_x_R.push_back(u_R*(E_R + p_R));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_x_R.push_back(u_R*(*(Z_R[si])));
                        }
                        
                        if (s_R <= 0)
                        {
                            F_x_intercell_HLL = F_x_R;
                        }
                        else
                        {
                            std::vector<const double*> Q_R;
                            for (int si = 0; si < d_num_species; si++)
                            {
                                Q_R.push_back(Z_rho_R[si]);
                            }
                            Q_R.push_back(m_R[0]);
                            Q_R.push_back(m_R[1]);
                            Q_R.push_back(m_R[2]);
                            Q_R.push_back(&E_R);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_R.push_back(Z_R[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_x_intercell_HLL.push_back((s_R*F_x_L[ei] - s_L*F_x_R[ei] +
                                    s_R*s_L*(*(Q_R[ei]) - *(Q_L[ei])))/(s_R - s_L));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_L + s_minus*((s_L - u_L)/(s_L - s_star) - 1);
                    v_intercell = v_L;
                    w_intercell = w_L;
                }
                else
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_R = (s_R - u_R)/(s_R - s_star);
                    
                    std::vector<double> Q_star_R;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_star_R.push_back(Chi_star_R*(*(Z_rho_R[si])));
                    }
                    Q_star_R.push_back(Chi_star_R*rho_R*s_star);
                    Q_star_R.push_back(Chi_star_R*rho_v_R);
                    Q_star_R.push_back(Chi_star_R*rho_w_R);
                    Q_star_R.push_back(Chi_star_R*(E_R + (s_star - u_R)*(rho_R*s_star + p_R/(s_R - u_R))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_R.push_back(Chi_star_R*(*(Z_R[si])));
                    }
                    
                    std::vector<const double*> Q_R;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_R.push_back(Z_rho_R[si]);
                    }
                    Q_R.push_back(m_R[0]);
                    Q_R.push_back(m_R[1]);
                    Q_R.push_back(m_R[2]);
                    Q_R.push_back(&E_R);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_R.push_back(Z_R[si]);
                    }
                    
                    std::vector<double> F_x_R;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        F_x_R.push_back(u_R*(*(Z_rho_R[si])));
                    }
                    F_x_R.push_back(rho_u_R*u_R + p_R);
                    F_x_R.push_back(rho_u_R*v_R);
                    F_x_R.push_back(rho_u_R*w_R);
                    F_x_R.push_back(u_R*(E_R + p_R));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_x_R.push_back(u_R*(*(Z_R[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_x_intercell_HLLC.push_back(F_x_R[ei] + s_plus*(Q_star_R[ei] - *(Q_R[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_R <= 0)
                    {
                        F_x_intercell_HLL = F_x_R;
                    }
                    else
                    {
                        std::vector<double> F_x_L;
                        for (int si = 0; si < d_num_species; si++)
                        {
                            F_x_L.push_back(u_L*(*(Z_rho_L[si])));
                        }
                        F_x_L.push_back(rho_u_L*u_L + p_L);
                        F_x_L.push_back(rho_u_L*v_L);
                        F_x_L.push_back(rho_u_L*w_L);
                        F_x_L.push_back(u_L*(E_L + p_L));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_x_L.push_back(u_L*(*(Z_L[si])));
                        }
                        
                        if (s_L >= 0)
                        {
                            F_x_intercell_HLL = F_x_L;
                        }
                        else
                        {
                            std::vector<const double*> Q_L;
                            for (int si = 0; si < d_num_species; si++)
                            {
                                Q_L.push_back(Z_rho_L[si]);
                            }
                            Q_L.push_back(m_L[0]);
                            Q_L.push_back(m_L[1]);
                            Q_L.push_back(m_L[2]);
                            Q_L.push_back(&E_L);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_L.push_back(Z_L[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_x_intercell_HLL.push_back((s_R*F_x_L[ei] - s_L*F_x_R[ei] +
                                    s_R*s_L*(*(Q_R[ei]) - *(Q_L[ei])))/(s_R - s_L));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_R + s_plus*((s_R - u_R)/(s_R - s_star) - 1);
                    v_intercell = v_R;
                    w_intercell = w_R;
                }
                
                /*
                 * Calulate the weights beta for hybridization.
                 */
                
                double alpha1, alpha2;
                
                double vel_mag = sqrt(pow(u_R - u_L, 2) + pow(v_R - v_L, 2) + pow(w_R - w_L, 2));
                
                if (vel_mag < 1e-40)
                {
                    alpha1 = 1.0;
                    alpha2 = 0.0;
                }
                else
                {
                    alpha1 = fabs(u_R - u_L)/vel_mag;
                    alpha2 = sqrt(1.0 - alpha1*alpha1);
                }
                
                double beta1 = 0.5 + 0.5*alpha1/(alpha1 + alpha2);
                double beta2 = 1.0 - beta1;
                
                /*
                 * Compute the HLLC-HLL flux.
                 */
                
                for (int si = 0; si < d_num_species; si++)
                {
                    *(F_x_intercell[si]) = beta1*F_x_intercell_HLLC[si] + beta2*F_x_intercell_HLL[si];
                }
                *(F_x_intercell[d_num_species]) = F_x_intercell_HLLC[d_num_species];
                *(F_x_intercell[d_num_species + 1]) = beta1*F_x_intercell_HLLC[d_num_species + 1] +
                    beta2*F_x_intercell_HLL[d_num_species + 1];
                *(F_x_intercell[d_num_species + 2]) = beta1*F_x_intercell_HLLC[d_num_species + 2] +
                    beta2*F_x_intercell_HLL[d_num_species + 2];
                *(F_x_intercell[d_num_species + 3]) = F_x_intercell_HLLC[d_num_species + 3];
                for (int si = 0; si < d_num_species - 1; si++)
                {
                    *(F_x_intercell[d_num_species + 4 + si]) = beta1*F_x_intercell_HLLC[d_num_species + 4 + si] +
                        beta2*F_x_intercell_HLL[d_num_species + 4 + si];
                    // *(F_x_intercell[d_num_species + 4 + si]) = F_x_intercell_HLLC[d_num_species + 4 + si];
                }
                
                break;
            }
            case Y_DIRECTION:
            {
                std::vector<double*> F_y_intercell = flux_intercell;
                
                std::vector<double> F_y_intercell_HLLC;
                std::vector<double> F_y_intercell_HLL;
                
                double& u_intercell = *(velocity_intercell[0]);
                double& v_intercell = *(velocity_intercell[1]);
                double& w_intercell = *(velocity_intercell[2]);
                
                const std::vector<const double*> Z_rho_B = partial_density_L;
                const std::vector<const double*> Z_rho_T = partial_density_R;
                
                const double rho_B = d_equation_of_state->getTotalDensity(
                    partial_density_L);
                
                const double rho_T = d_equation_of_state->getTotalDensity(
                    partial_density_R);
                
                const double& rho_u_B = *(momentum_L[0]);
                const double& rho_u_T = *(momentum_R[0]);
                
                const double& rho_v_B = *(momentum_L[1]);
                const double& rho_v_T = *(momentum_R[1]);
                
                const double& rho_w_B = *(momentum_L[2]);
                const double& rho_w_T = *(momentum_R[2]);
                
                const std::vector<const double*> m_B = momentum_L;
                const std::vector<const double*> m_T = momentum_R;
                
                const double& u_B = rho_u_B/rho_B;
                const double& u_T = rho_u_T/rho_T;
                
                const double& v_B = rho_v_B/rho_B;
                const double& v_T = rho_v_T/rho_T;
                
                const double& w_B = rho_w_B/rho_B;
                const double& w_T = rho_w_T/rho_T;
                
                const double& E_B = *total_energy_L;
                const double& E_T = *total_energy_R;
                
                const std::vector<const double*> Z_B = volume_fraction_L;
                const std::vector<const double*> Z_T = volume_fraction_R;
                
                const double p_B = d_equation_of_state->getPressureWithVolumeFraction(
                    &rho_B,
                    m_B,
                    &E_B,
                    Z_B);
                
                const double p_T = d_equation_of_state->getPressureWithVolumeFraction(
                    &rho_T,
                    m_T,
                    &E_T,
                    Z_T);
                
                const double c_B = d_equation_of_state->getSoundSpeedWithVolumeFractionAndPressure(
                    &rho_B,
                    Z_B,
                    &p_B);
                
                const double c_T = d_equation_of_state->getSoundSpeedWithVolumeFractionAndPressure(
                    &rho_T,
                    Z_T,
                    &p_T);
                
                const double v_average = 0.5*(v_B + v_T);
                const double c_average = 0.5*(c_B + c_T);
                
                const double s_B = fmin(v_average - c_average, v_B - c_B);
                const double s_T = fmax(v_average + c_average, v_T + c_T);
                
                const double s_minus = fmin(0.0, s_B);
                const double s_plus  = fmax(0.0, s_T);
                
                const double s_star =
                    (p_T - p_B + rho_B*v_B*(s_B - v_B) - rho_T*v_T*(s_T - v_T))/(rho_B*(s_B - v_B) -
                        rho_T*(s_T - v_T));
                
                if (s_star > 0)
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_B = (s_B - v_B)/(s_B - s_star);    
                    
                    std::vector<double> Q_star_B;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_star_B.push_back(Chi_star_B*(*(Z_rho_B[si])));
                    }
                    Q_star_B.push_back(Chi_star_B*rho_u_B);
                    Q_star_B.push_back(Chi_star_B*rho_B*s_star);
                    Q_star_B.push_back(Chi_star_B*rho_w_B);
                    Q_star_B.push_back(Chi_star_B*(E_B + (s_star - v_B)*(rho_B*s_star + p_B/(s_B - v_B))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_B.push_back(Chi_star_B*(*(Z_B[si])));
                    }
                    
                    std::vector<const double*> Q_B;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_B.push_back(Z_rho_B[si]);
                    }
                    Q_B.push_back(m_B[0]);
                    Q_B.push_back(m_B[1]);
                    Q_B.push_back(m_B[2]);
                    Q_B.push_back(&E_B);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_B.push_back(Z_B[si]);
                    }
                    
                    std::vector<double> F_y_B;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        F_y_B.push_back(v_B*(*(Z_rho_B[si])));
                    }
                    F_y_B.push_back(rho_v_B*u_B);
                    F_y_B.push_back(rho_v_B*v_B + p_B);
                    F_y_B.push_back(rho_v_B*w_B);
                    F_y_B.push_back(v_B*(E_B + p_B));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_y_B.push_back(v_B*(*(Z_B[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_y_intercell_HLLC.push_back(F_y_B[ei] + s_minus*(Q_star_B[ei] - *(Q_B[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_B >= 0)
                    {
                        F_y_intercell_HLL = F_y_B;
                    }
                    else
                    {
                        std::vector<double> F_y_T;
                        for (int si = 0; si < d_num_species; si++)
                        {
                            F_y_T.push_back(v_T*(*(Z_rho_T[si])));
                        }
                        F_y_T.push_back(rho_v_T*u_T);
                        F_y_T.push_back(rho_v_T*v_T + p_T);
                        F_y_T.push_back(rho_v_T*w_T);
                        F_y_T.push_back(v_T*(E_T + p_T));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_y_T.push_back(v_T*(*(Z_T[si])));
                        }
                        
                        if (s_T <= 0)
                        {
                            F_y_intercell_HLL = F_y_T;
                        }
                        else
                        {
                            std::vector<const double*> Q_T;
                            for (int si = 0; si < d_num_species; si++)
                            {
                                Q_T.push_back(Z_rho_T[si]);
                            }
                            Q_T.push_back(m_T[0]);
                            Q_T.push_back(m_T[1]);
                            Q_T.push_back(m_T[2]);
                            Q_T.push_back(&E_T);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_T.push_back(Z_T[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_y_intercell_HLL.push_back((s_T*F_y_B[ei] - s_B*F_y_T[ei] +
                                    s_T*s_B*(*(Q_T[ei]) - *(Q_B[ei])))/(s_T - s_B));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_B;
                    v_intercell = v_B + s_minus*((s_B - v_B)/(s_B - s_star) - 1);
                    w_intercell = w_B;
                }
                else
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_T = (s_T - v_T)/(s_T - s_star); 
                    
                    std::vector<double> Q_star_T;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_star_T.push_back(Chi_star_T*(*(Z_rho_T[si])));
                    }
                    Q_star_T.push_back(Chi_star_T*rho_u_T);
                    Q_star_T.push_back(Chi_star_T*rho_T*s_star);
                    Q_star_T.push_back(Chi_star_T*rho_w_T);
                    Q_star_T.push_back(Chi_star_T*(E_T + (s_star - v_T)*(rho_T*s_star + p_T/(s_T - v_T))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_T.push_back(Chi_star_T*(*(Z_T[si])));
                    }
                    
                    std::vector<const double*> Q_T;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_T.push_back(Z_rho_T[si]);
                    }
                    Q_T.push_back(m_T[0]);
                    Q_T.push_back(m_T[1]);
                    Q_T.push_back(m_T[2]);
                    Q_T.push_back(&E_T);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_T.push_back(Z_T[si]);
                    }
                    
                    std::vector<double> F_y_T;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        F_y_T.push_back(v_T*(*(Z_rho_T[si])));
                    }
                    F_y_T.push_back(rho_v_T*u_T);
                    F_y_T.push_back(rho_v_T*v_T + p_T);
                    F_y_T.push_back(rho_v_T*w_T);
                    F_y_T.push_back(v_T*(E_T + p_T));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_y_T.push_back(v_T*(*(Z_T[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_y_intercell_HLLC.push_back(F_y_T[ei] + s_plus*(Q_star_T[ei] - *(Q_T[ei])));
                    }
                    
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    if (s_T <= 0)
                    {
                        F_y_intercell_HLL = F_y_T;
                    }
                    else
                    {
                        std::vector<double> F_y_B;
                        for (int si = 0; si < d_num_species; si++)
                        {
                            F_y_B.push_back(v_B*(*(Z_rho_B[si])));
                        }
                        F_y_B.push_back(rho_v_B*u_B);
                        F_y_B.push_back(rho_v_B*v_B + p_B);
                        F_y_B.push_back(rho_v_B*w_B);
                        F_y_B.push_back(v_B*(E_B + p_B));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_y_B.push_back(v_B*(*(Z_B[si])));
                        }
                        
                        if (s_B >= 0)
                        {
                            F_y_intercell_HLL = F_y_B;
                        }
                        else
                        {
                            std::vector<const double*> Q_B;
                            for (int si = 0; si < d_num_species; si++)
                            {
                                Q_B.push_back(Z_rho_B[si]);
                            }
                            Q_B.push_back(m_B[0]);
                            Q_B.push_back(m_B[1]);
                            Q_B.push_back(m_B[2]);
                            Q_B.push_back(&E_B);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_B.push_back(Z_B[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_y_intercell_HLL.push_back((s_T*F_y_B[ei] - s_B*F_y_T[ei] +
                                    s_T*s_B*(*(Q_T[ei]) - *(Q_B[ei])))/(s_T - s_B));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_T;
                    v_intercell = v_T + s_plus*((s_T - v_T)/(s_T - s_star) - 1);
                    w_intercell = w_T;
                }
                
                /*
                 * Calulate the weights beta for hybridization.
                 */
                
                double alpha1, alpha2;
                
                double vel_mag = sqrt(pow(u_T - u_B, 2) + pow(v_T - v_B, 2) + pow(w_T - w_B, 2));
                
                if (vel_mag < 1e-40)
                {
                    alpha1 = 1.0;
                    alpha2 = 0.0;
                }
                else
                {
                    alpha1 = fabs(v_T - v_B)/vel_mag;
                    alpha2 = sqrt(1.0 - alpha1*alpha1);
                }
                
                double beta1 = 0.5 + 0.5*alpha1/(alpha1 + alpha2);
                double beta2 = 1.0 - beta1;
                
                /*
                 * Compute the HLLC-HLL flux.
                 */
                
                for (int si = 0; si < d_num_species; si++)
                {
                    *(F_y_intercell[si]) = beta1*F_y_intercell_HLLC[si] + beta2*F_y_intercell_HLL[si];
                }
                *(F_y_intercell[d_num_species]) = beta1*F_y_intercell_HLLC[d_num_species] +
                    beta2*F_y_intercell_HLL[d_num_species];
                *(F_y_intercell[d_num_species + 1]) = F_y_intercell_HLLC[d_num_species + 1];
                *(F_y_intercell[d_num_species + 2]) = beta1*F_y_intercell_HLLC[d_num_species + 2] +
                    beta2*F_y_intercell_HLL[d_num_species + 2];
                *(F_y_intercell[d_num_species + 3]) = F_y_intercell_HLLC[d_num_species + 3];
                for (int si = 0; si < d_num_species - 1; si++)
                {
                    *(F_y_intercell[d_num_species + 4 + si]) = beta1*F_y_intercell_HLLC[d_num_species + 4 + si] +
                        beta2*F_y_intercell_HLL[d_num_species + 4 + si];
                    // *(F_y_intercell[d_num_species + 4 + si]) = F_y_intercell_HLLC[d_num_species + 4 + si];
                }
                
                break;
            }
            case Z_DIRECTION:
            {
                std::vector<double*> F_z_intercell = flux_intercell;
                
                std::vector<double> F_z_intercell_HLLC;
                std::vector<double> F_z_intercell_HLL;
                
                double& u_intercell = *(velocity_intercell[0]);
                double& v_intercell = *(velocity_intercell[1]);
                double& w_intercell = *(velocity_intercell[2]);
                
                const std::vector<const double*> Z_rho_B = partial_density_L;
                const std::vector<const double*> Z_rho_F = partial_density_R;
                
                const double rho_B = d_equation_of_state->getTotalDensity(
                    partial_density_L);
                
                const double rho_F = d_equation_of_state->getTotalDensity(
                    partial_density_R);
                
                const double& rho_u_B = *(momentum_L[0]);
                const double& rho_u_F = *(momentum_R[0]);
                
                const double& rho_v_B = *(momentum_L[1]);
                const double& rho_v_F = *(momentum_R[1]);
                
                const double& rho_w_B = *(momentum_L[2]);
                const double& rho_w_F = *(momentum_R[2]);
                
                const std::vector<const double*> m_B = momentum_L;
                const std::vector<const double*> m_F = momentum_R;
                
                const double& u_B = rho_u_B/rho_B;
                const double& u_F = rho_u_F/rho_F;
                
                const double& v_B = rho_v_B/rho_B;
                const double& v_F = rho_v_F/rho_F;
                
                const double& w_B = rho_w_B/rho_B;
                const double& w_F = rho_w_F/rho_F;
                
                const double& E_B = *total_energy_L;
                const double& E_F = *total_energy_R;
                
                const std::vector<const double*> Z_B = volume_fraction_L;
                const std::vector<const double*> Z_F = volume_fraction_R;
                
                const double p_B = d_equation_of_state->getPressureWithVolumeFraction(
                    &rho_B,
                    m_B,
                    &E_B,
                    Z_B);
                
                const double p_F = d_equation_of_state->getPressureWithVolumeFraction(
                    &rho_F,
                    m_F,
                    &E_F,
                    Z_F);
                
                const double c_B = d_equation_of_state->getSoundSpeedWithVolumeFractionAndPressure(
                    &rho_B,
                    Z_B,
                    &p_B);
                
                const double c_F = d_equation_of_state->getSoundSpeedWithVolumeFractionAndPressure(
                    &rho_F,
                    Z_F,
                    &p_F);
                
                const double w_average = 0.5*(w_B + w_F);
                const double c_average = 0.5*(c_F + c_F);
                
                const double s_B = fmin(w_average - c_average, w_B - c_B);
                const double s_F = fmax(w_average + c_average, w_F + c_F);
                
                const double s_minus = fmin(0.0, s_B);
                const double s_plus  = fmax(0.0, s_F);
                
                const double s_star =
                    (p_F - p_B + rho_B*w_B*(s_B - w_B) - rho_F*w_F*(s_F - w_F))/(rho_B*(s_B - w_B) -
                        rho_F*(s_F - w_F));
                
                if (s_star > 0)
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_B = (s_B - w_B)/(s_B - s_star);    
                    
                    std::vector<double> Q_star_B;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_star_B.push_back(Chi_star_B*(*(Z_rho_B[si])));
                    }
                    Q_star_B.push_back(Chi_star_B*rho_u_B);
                    Q_star_B.push_back(Chi_star_B*rho_v_B);
                    Q_star_B.push_back(Chi_star_B*rho_B*s_star);
                    Q_star_B.push_back(Chi_star_B*(E_B + (s_star - w_B)*(rho_B*s_star + p_B/(s_B - w_B))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_B.push_back(Chi_star_B*(*(Z_B[si])));
                    }
                    
                    std::vector<const double*> Q_B;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_B.push_back(Z_rho_B[si]);
                    }
                    Q_B.push_back(m_B[0]);
                    Q_B.push_back(m_B[1]);
                    Q_B.push_back(m_B[2]);
                    Q_B.push_back(&E_B);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_B.push_back(Z_B[si]);
                    }
                    
                    std::vector<double> F_z_B;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        F_z_B.push_back(w_B*(*(Z_rho_B[si])));
                    }
                    F_z_B.push_back(rho_w_B*u_B);
                    F_z_B.push_back(rho_w_B*v_B);
                    F_z_B.push_back(rho_w_B*w_B + p_B);
                    F_z_B.push_back(w_B*(E_B + p_B));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_z_B.push_back(w_B*(*(Z_B[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_z_intercell_HLLC.push_back(F_z_B[ei] + s_minus*(Q_star_B[ei] - *(Q_B[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_B >= 0)
                    {
                        F_z_intercell_HLL = F_z_B;
                    }
                    else
                    {
                        std::vector<double> F_z_F;
                        for (int si = 0; si < d_num_species; si++)
                        {
                            F_z_F.push_back(w_F*(*(Z_rho_F[si])));
                        }
                        F_z_F.push_back(rho_w_F*u_F);
                        F_z_F.push_back(rho_w_F*v_F);
                        F_z_F.push_back(rho_w_F*w_F + p_F);
                        F_z_F.push_back(w_F*(E_F + p_F));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_z_F.push_back(w_F*(*(Z_F[si])));
                        }
                        
                        if (s_F <= 0)
                        {
                            F_z_intercell_HLL = F_z_F;
                        }
                        else
                        {
                            std::vector<const double*> Q_F;
                            for (int si = 0; si < d_num_species; si++)
                            {
                                Q_F.push_back(Z_rho_F[si]);
                            }
                            Q_F.push_back(m_F[0]);
                            Q_F.push_back(m_F[1]);
                            Q_F.push_back(m_F[2]);
                            Q_F.push_back(&E_F);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_F.push_back(Z_F[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_z_intercell_HLL.push_back((s_F*F_z_B[ei] - s_B*F_z_F[ei] +
                                    s_F*s_B*(*(Q_F[ei]) - *(Q_B[ei])))/(s_F - s_B));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_B;
                    v_intercell = v_B;
                    w_intercell = w_B + s_minus*((s_B - w_B)/(s_B - s_star) - 1);
                }
                else
                {
                    /*
                     * Compute the HLLC flux.
                     */
                    
                    const double Chi_star_F = (s_F - w_F)/(s_F - s_star); 
                    
                    std::vector<double> Q_star_F;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_star_F.push_back(Chi_star_F*(*(Z_rho_F[si])));
                    }
                    Q_star_F.push_back(Chi_star_F*rho_u_F);
                    Q_star_F.push_back(Chi_star_F*rho_v_F);
                    Q_star_F.push_back(Chi_star_F*rho_F*s_star);
                    Q_star_F.push_back(Chi_star_F*(E_F + (s_star - w_F)*(rho_F*s_star + p_F/(s_F - w_F))));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_star_F.push_back(Chi_star_F*(*(Z_F[si])));
                    }
                    
                    std::vector<const double*> Q_F;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Q_F.push_back(Z_rho_F[si]);
                    }
                    Q_F.push_back(m_F[0]);
                    Q_F.push_back(m_F[1]);
                    Q_F.push_back(m_F[2]);
                    Q_F.push_back(&E_F);
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        Q_F.push_back(Z_F[si]);
                    }
                    
                    std::vector<double> F_z_F;
                    for (int si = 0; si < d_num_species; si++)
                    {
                        F_z_F.push_back(w_F*(*(Z_rho_F[si])));
                    }
                    F_z_F.push_back(rho_w_F*u_F);
                    F_z_F.push_back(rho_w_F*v_F);
                    F_z_F.push_back(rho_w_F*w_F + p_F);
                    F_z_F.push_back(w_F*(E_F + p_F));
                    for (int si = 0; si < d_num_species - 1; si++)
                    {
                        F_z_F.push_back(w_F*(*(Z_F[si])));
                    }
                    
                    for (int ei = 0; ei < d_num_eqn; ei++)
                    {
                        F_z_intercell_HLLC.push_back(F_z_F[ei] + s_plus*(Q_star_F[ei] - *(Q_F[ei])));
                    }
                    
                    /*
                     * Compute the HLL flux.
                     */
                    
                    if (s_F <= 0)
                    {
                        F_z_intercell_HLL = F_z_F;
                    }
                    else
                    {
                        std::vector<double> F_z_B;
                        for (int si = 0; si < d_num_species; si++)
                        {
                            F_z_B.push_back(w_B*(*(Z_rho_B[si])));
                        }
                        F_z_B.push_back(rho_w_B*u_B);
                        F_z_B.push_back(rho_w_B*v_B);
                        F_z_B.push_back(rho_w_B*w_B + p_B);
                        F_z_B.push_back(w_B*(E_B + p_B));
                        for (int si = 0; si < d_num_species - 1; si++)
                        {
                            F_z_B.push_back(w_B*(*(Z_B[si])));
                        }
                        
                        if (s_B >= 0)
                        {
                            F_z_intercell_HLL = F_z_B;
                        }
                        else
                        {
                            std::vector<const double*> Q_B;
                            for (int si = 0; si < d_num_species; si++)
                            {
                                Q_B.push_back(Z_rho_B[si]);
                            }
                            Q_B.push_back(m_B[0]);
                            Q_B.push_back(m_B[1]);
                            Q_B.push_back(m_B[2]);
                            Q_B.push_back(&E_B);
                            for (int si = 0; si < d_num_species - 1; si++)
                            {
                                Q_B.push_back(Z_B[si]);
                            }
                            
                            for (int ei = 0; ei < d_num_eqn; ei++)
                            {
                                F_z_intercell_HLL.push_back((s_F*F_z_B[ei] - s_B*F_z_F[ei] +
                                    s_F*s_B*(*(Q_F[ei]) - *(Q_B[ei])))/(s_F - s_B));
                            }
                        }
                    }
                    
                    /*
                     * Compute the intercell velocity.
                     */
                    
                    u_intercell = u_F;
                    v_intercell = v_F;
                    w_intercell = w_F + s_plus*((s_F - w_F)/(s_F - s_star) - 1);
                }
                
                /*
                 * Calulate the weights beta for hybridization.
                 */
                
                double alpha1, alpha2;
                
                double vel_mag = sqrt(pow(u_F - u_B, 2) + pow(v_F - v_B, 2) + pow(w_F - w_B, 2));
                
                if (vel_mag < 1e-40)
                {
                    alpha1 = 1.0;
                    alpha2 = 0.0;
                }
                else
                {
                    alpha1 = fabs(w_F - w_B)/vel_mag;
                    alpha2 = sqrt(1.0 - alpha1*alpha1);
                }
                
                double beta1 = 0.5 + 0.5*alpha1/(alpha1 + alpha2);
                double beta2 = 1.0 - beta1;
                
                /*
                 * Compute the HLLC-HLL flux.
                 */
                
                for (int si = 0; si < d_num_species; si++)
                {
                    *(F_z_intercell[si]) = beta1*F_z_intercell_HLLC[si] + beta2*F_z_intercell_HLL[si];
                }
                *(F_z_intercell[d_num_species]) = beta1*F_z_intercell_HLLC[d_num_species] +
                    beta2*F_z_intercell_HLL[d_num_species];
                *(F_z_intercell[d_num_species + 1]) = beta1*F_z_intercell_HLLC[d_num_species + 1] +
                    beta2*F_z_intercell_HLL[d_num_species + 1];
                *(F_z_intercell[d_num_species + 2]) = F_z_intercell_HLLC[d_num_species + 2];
                *(F_z_intercell[d_num_species + 3]) = F_z_intercell_HLLC[d_num_species + 3];
                for (int si = 0; si < d_num_species - 1; si++)
                {
                    *(F_z_intercell[d_num_species + 4 + si]) = beta1*F_z_intercell_HLLC[d_num_species + 4 + si] +
                        beta2*F_z_intercell_HLL[d_num_species + 4 + si];
                    // *(F_z_intercell[d_num_species + 4 + si]) = F_z_intercell_HLLC[d_num_species + 4 + si];
                }
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                           << ": "
                           << "Unknown direction."
                           << std::endl);
            }
        }
    }
}
